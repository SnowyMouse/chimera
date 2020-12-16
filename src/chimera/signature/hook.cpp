// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <iostream>
#include <cstring>
#include <string>

#include "signature.hpp"
#include "hook.hpp"

namespace Chimera {
    void Hook::rollback() noexcept {
        if(this->original_bytes.size() == 0) {
            return;
        }
        overwrite(this->address, this->original_bytes.data(), this->original_bytes.size());
        this->original_bytes.clear();
    }

    // Get the bytes to the instruction(s) at the given address. I'll modify this as more types of instructions are needed.
    void get_instructions(const std::byte *at_start, std::vector<std::byte> &bytes, std::vector<std::uintptr_t> &offsets, std::size_t minimum_size = 1) {
        offsets.clear();
        const auto *at = at_start;

        // Keep adding instructions until we have what we need.
        while(bytes.size() < minimum_size) {
            switch(*reinterpret_cast<const std::uint8_t *>(at)) {
                // add eax, <val>
                case 0x05:
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 5);
                    at += 5;
                    break;

                // jmp <relative offset> or movsx
                case 0x0F: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    auto op2 = *reinterpret_cast<const std::uint8_t *>(at + 2);
                    if(op1 == 0x84) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                        break;
                    }
                    else if(op1 == 0xBF || op1 == 0xB6 || op1 == 0xB7) {
                        if(op2 == 0x6E || op2 == 0x4E) {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 4);
                            at += 4;
                        }
                        else if(op2 == 0x15) {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 7);
                            at += 7;
                        }
                        else {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 3);
                            at += 3;
                        }
                        break;
                    }
                    else {
                        std::terminate();
                    }
                }

                // and <value>
                case 0x25: {
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 5);
                    at += 5;
                    break;
                }

                // oxr <value>
                case 0x33: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(op1 == 0xDB) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }
                    std::terminate();
                }

                // cmp ecx, something
                case 0x3B: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(op1 == 0xCD) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }
                    std::terminate();
                }


                // push/pop <register>
                case 0x50: case 0x54: case 0x58: case 0x5C: case 0x60:
                case 0x51: case 0x55: case 0x59: case 0x5D: case 0x61:
                case 0x52: case 0x56: case 0x5A: case 0x5E:
                case 0x53: case 0x57: case 0x5B: case 0x5F:
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 1);
                    at += 1;
                    break;

                case 0x66: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    auto op2 = *reinterpret_cast<const std::uint8_t *>(at + 2);
                    // mov [reg]
                    if(op1 == 0x89) {
                        if(op2 == 0x45 || op2 == 0x4A) {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 4);
                            at += 4;
                        }
                        else {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 3);
                            at += 3;
                        }
                        break;
                    }
                    // mov word ptr [reg+op4]
                    else if(op1 == 0xC7) {
                        if(op2 == 0x45) {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 6);
                            at += 6;
                        }
                        else if(op2 == 0x44) {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 7);
                            at += 7;
                        }
                        break;
                    }
                    // sub dword ptr [reg+op4], reg
                    else if(op1 == 0x29) {
                        if(op2 == 0x8B) {
                            offsets.push_back(at - at_start);
                            bytes.insert(bytes.end(), at, at + 7);
                            at += 7;
                        }
                        break;
                    }
                    // mov [addr], ax
                    else if(op1 == 0xA3) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                        break;
                    }
                    // cmp reg, [reg+op3] or mov reg, [reg+op3]
                    else if(op1 == 0x3B || op1 == 0x3D || op1 == 0x8B) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 4);
                        at += 4;
                        break;
                    }
                    std::terminate();
                }

                // push 0x00000000-0xFFFFFFFF
                case 0x68: {
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 5);
                    at += 5;
                    break;
                }

                // push 0x00000000-0xFFFFFFFF
                case 0x69: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(op1 == 0xFF) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                        break;
                    }
                    std::terminate();
                }

                // push 0x00-0xFF
                case 0x6A: {
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 2);
                    at += 2;
                    break;
                }

                // jz short
                case 0x74: {
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 2);
                    at += 2;
                    break;
                }

                // jmp 0x00-0x7F
                case 0x7D: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(op1 < 0x80) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }
                    std::terminate();
                }

                // add/or/adc/sbb/and/sub/xor/cmp <something> 0x00000000-0x7FFFFFFF
                case 0x81: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    // add/or/adc/sbb/and/sub/xor/cmp <register> 0x00000000-0x7FFFFFFF
                    if(op1 >= 0xC0 || op1 == 0x0D) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                    }
                    else {
                        std::terminate();
                    }
                    break;
                }

                // add/or/adc/sbb/and/sub/xor/cmp <something> 0x00-0x7F
                case 0x83: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    // add/or/adc/sbb/and/sub/xor/cmp <register> 0x00 - 0x7F
                    if(op1 >= 0xC0) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 3);
                        at += 3;
                    }
                    else {
                        std::terminate();
                    }
                    break;
                }

                // test <something>, <something>
                case 0x84:
                case 0x85: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    // add/or/adc/sbb/and/sub/xor/cmp <register> 0x00 - 0x7F
                    if(op1 >= 0xC0) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }
                    else {
                        std::terminate();
                    }
                }

                // idk
                case 0x89: {
                    auto a = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(a == 0x06) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }

                    if(a == 0x15 || a == 0x3D) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                        break;
                    }

                    if(a == 0x6C || a == 0x4C) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 4);
                        at += 4;
                        break;
                    }

                    auto b = *reinterpret_cast<const std::uint8_t *>(at + 2);
                    if(a == 0xC && b == 0x85) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 7);
                        at += 7;
                        break;
                    }

                    if((a == 0x94 || a == 0x84) && b == 0x24) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 7);
                        at += 7;
                        break;
                    }

                    std::terminate();
                }

                // mov bl, [eax+esi]
                case 0x8A: {
                    auto a = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(a == 0x1C || a == 0x48) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 3);
                        at += 3;
                        break;
                    }

                    std::terminate();
                }

                // moving stuff
                case 0x8B: {
                    auto a = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    auto b = *reinterpret_cast<const std::uint8_t *>(at + 2);
                    if((a == 0x6C || a == 0x4C || a == 0x44 || a == 0x54) && b == 0x24) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 4);
                        at += 4;
                        break;
                    }
                    else if(a == 0xE5 || a == 0xF8 || a == 0xC3 || a == 0xC2 || a == 0xEC) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }
                    else if(a == 0x50 || a == 0x40) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 3);
                        at += 3;
                        break;
                    }
                    else if(a == 0x93 || a == 0x0D || a == 0x2D || a == 0x1D || a == 0x83) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                        break;
                    }
                    std::terminate();
                }

                // lea
                case 0x8D: {
                    auto a = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    auto b = *reinterpret_cast<const std::uint8_t *>(at + 2);
                    if(a == 0x44 && (b == 0x0C || b == 0x24)) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 4);
                        at += 4;
                        break;
                    }
                    else if(a == 0x7E) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 3);
                        at += 3;
                        break;
                    }

                    std::terminate();
                }

                // shl
                case 0xD3: {
                    auto a = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(a == 0xE3) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }

                    std::terminate();
                }

                // nop
                case 0x90: {
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 1);
                    at ++;
                    break;
                }

                // mov al, byte [something]
                case 0xA0:
                // mov eax, dword [something]
                case 0xA1:
                // move byte [something], al
                case 0xA2:
                // move dword [something], eax
                case 0xA3: {
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 5);
                    at += 5;
                    break;
                }

                // mov something
                case 0xB8:
                case 0xBA:
                case 0xBB:
                case 0xBE: {
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 5);
                    at += 5;
                    break;
                }

                // mov something
                case 0xC7: {
                    auto a = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(a == 0x05) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                        break;
                    }
                    else if(a == 0x44) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 8);
                        at += 8;
                        break;
                    }
                    std::terminate();
                }

                // fmul
                case 0xD8: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(op1 == 0x4F) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 3);
                        at += 3;
                        break;
                    }
                    std::terminate();
                }

                // fld / fst
                case 0xD9: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(op1 == 0x47 || op1 == 0x55) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 3);
                        at += 3;
                        break;
                    }
                    else if(op1 == 0xC0) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                        break;
                    }
                    else if(op1 == 0x1C || op1 == 0x9C) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 7);
                        at += 7;
                        break;
                    }
                    std::terminate();
                }

                // call <relative offset>
                case 0xE8:
                    offsets.push_back(at - at_start);
                    bytes.insert(bytes.end(), at, at + 5);
                    at += 5;
                    break;

                // call dword ptr[x]
                case 0xFF: {
                    auto op1 = *reinterpret_cast<const std::uint8_t *>(at + 1);
                    if(op1 == 0x51 || op1 == 0x52) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 3);
                        at += 3;
                    }
                    else if(op1 == 0xD3) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 2);
                        at += 2;
                    }
                    else if(op1 == 0x54) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 4);
                        at += 4;
                    }
                    else if(op1 == 0x15 || op1 == 0x92) {
                        offsets.push_back(at - at_start);
                        bytes.insert(bytes.end(), at, at + 6);
                        at += 6;
                    }
                    else {
                        std::terminate();
                    }
                    break;
                }

                // Terminate. We don't know what to do.
                default:
                    std::cout << "Cannot figure out what's at " << std::to_string(reinterpret_cast<std::uintptr_t>(at)) << std::endl;
                    std::terminate();
            }
        }
    }

    void write_jmp_call(void *jmp_at, Hook &hook, const void *call_before, const void *call_after, bool pushad_pushfd) {
        // Rollback the hook if not already done so
        hook.rollback();

        // Write the address for the hook.
        hook.address = reinterpret_cast<std::byte *>(jmp_at);

        // Get the instructions
        std::vector<std::uintptr_t> offsets;
        std::vector<std::byte> bytes;
        std::byte *jmp_at_byte = reinterpret_cast<std::byte *>(jmp_at);
        get_instructions(jmp_at_byte, bytes, offsets, 5);

        // Calculate how much data we'll need. (size of bytes plus 9 bytes per call [5 for the call and 4 for pushad/popad and pushfd/popfd])
        std::size_t added_pushad_bytes = pushad_pushfd ? 4 : 0;
        std::size_t size = bytes.size() + (call_before ? 5 + added_pushad_bytes : 0) + (call_after ? 5 + added_pushad_bytes : 0) + 5;

        // Back up the original bytes
        hook.original_bytes.insert(hook.original_bytes.end(), jmp_at_byte, jmp_at_byte + bytes.size());

        // Now make the hook
        hook.hook = std::make_unique<std::byte []>(size);
        auto *hook_data = hook.hook.get();
        DWORD old_protection;

        // Give it PAGE_EXECUTE_READWRITE so the Discord overlay doesn't crash Halo
        VirtualProtect(hook_data, size, PAGE_EXECUTE_READWRITE, &old_protection);

        // Overwrite the original bytes with NOPs and a jmp instruction
        DWORD new_protection = PAGE_EXECUTE_READWRITE;
        VirtualProtect(jmp_at_byte, bytes.size(), new_protection, &old_protection);
        *reinterpret_cast<std::uint8_t *>(jmp_at_byte) = 0xE9;
        *reinterpret_cast<std::uintptr_t *>(jmp_at_byte + 1) = hook_data - (jmp_at_byte + 5);
        std::memset(jmp_at_byte + 5, 0x90, bytes.size() - 5);
        if(old_protection != new_protection) {
            VirtualProtect(jmp_at, bytes.size(), old_protection, &new_protection);
        }

        // Let's do dis
        auto add_call = [&pushad_pushfd](const void *where, std::byte *data) {
            std::size_t call_offset = pushad_pushfd ? 2 : 0;

            if(pushad_pushfd) {
                // pushfd
                *reinterpret_cast<std::uint8_t *>(data + 0) = 0x9C;
                // pushad
                *reinterpret_cast<std::uint8_t *>(data + 1) = 0x60;

                // popad
                *reinterpret_cast<std::uint8_t *>(data + 7) = 0x61;
                // popfd
                *reinterpret_cast<std::uint8_t *>(data + 8) = 0x9D;
            }

            // call
            *reinterpret_cast<std::uint8_t *>(data + call_offset) = 0xE8;
            *reinterpret_cast<std::uintptr_t *>(data + call_offset + 1) = reinterpret_cast<const std::byte *>(where) - (data + call_offset + 5);
        };

        // Add the first call
        if(call_before) {
            add_call(reinterpret_cast<const std::uint8_t *>(call_before), hook_data);
            hook_data += 5 + added_pushad_bytes;
        }

        // Copy the original instructions
        std::copy(bytes.data(), bytes.data() + bytes.size(), hook_data);

        // Look for any call instructions to modify before proceeding
        for(const std::uintptr_t &offset : offsets) {
            if(*reinterpret_cast<std::uint8_t *>(hook_data + offset) == 0xE8) {
                // Find where this call instruction is calling.
                auto &op = *reinterpret_cast<std::uintptr_t *>(hook_data + offset + 1);
                const auto *actual_address = (jmp_at_byte + offset + 5) + op;

                // Update the instruction
                op = reinterpret_cast<std::uintptr_t>(actual_address) - reinterpret_cast<std::uintptr_t>(hook_data + offset + 5);
            }
        }
        hook_data += bytes.size();

        // Add the other call
        if(call_after) {
            add_call(reinterpret_cast<const std::uint8_t *>(call_after), hook_data);
            hook_data += 5 + added_pushad_bytes;
        }

        // Add the jmp instruction to exit this hook
        *reinterpret_cast<std::uint8_t *>(hook_data) = 0xE9;
        *reinterpret_cast<std::uintptr_t *>(hook_data + 1) = (jmp_at_byte + bytes.size()) - (hook_data + 5);
    }

    void write_function_override(void *jmp_at, Hook &hook, const void *new_function, const void **original_function) {
        // Rollback the hook if not already done so
        hook.rollback();

        // Write the address for the hook.
        hook.address = reinterpret_cast<std::byte *>(jmp_at);

        // Get the instructions
        std::vector<std::uintptr_t> offsets;
        std::vector<std::byte> bytes;
        std::byte *jmp_at_byte = reinterpret_cast<std::byte *>(jmp_at);
        get_instructions(jmp_at_byte, bytes, offsets, 5);

        // Calculate how much data we'll need. (five bytes for jmping to new_function, the size of bytes, and five bytes to jmp back to the original function)
        std::size_t size = 5 + bytes.size() + 5;

        // Back up the original bytes
        hook.original_bytes.insert(hook.original_bytes.end(), jmp_at_byte, jmp_at_byte + bytes.size());

        // Now make the hook
        hook.hook = std::make_unique<std::byte []>(size);
        auto *hook_data = hook.hook.get();
        DWORD old_protection;

        // Give it PAGE_EXECUTE_READWRITE so the Discord overlay doesn't crash Halo
        VirtualProtect(hook_data, size, PAGE_EXECUTE_READWRITE, &old_protection);

        // Overwrite the original bytes with NOPs and a jmp instruction
        DWORD new_protection = PAGE_EXECUTE_READWRITE;
        VirtualProtect(jmp_at_byte, bytes.size(), new_protection, &old_protection);
        *reinterpret_cast<std::uint8_t *>(jmp_at_byte) = 0xE9;
        *reinterpret_cast<std::uintptr_t *>(jmp_at_byte + 1) = hook_data - (jmp_at_byte + 5);
        std::memset(jmp_at_byte + 5, 0x90, bytes.size() - 5);
        if(old_protection != new_protection) {
            VirtualProtect(jmp_at, bytes.size(), old_protection, &new_protection);
        }

        // Write a jmp to the new function
        *reinterpret_cast<std::uint8_t *>(hook_data) = 0xE9;
        *reinterpret_cast<std::uintptr_t *>(hook_data + 1) = reinterpret_cast<const std::byte *>(new_function) - (hook_data + 5);
        hook_data += 5;

        // Copy the original instructions
        std::copy(bytes.data(), bytes.data() + bytes.size(), hook_data);
        *original_function = hook_data;

        // Look for any call instructions to modify before proceeding
        for(const std::uintptr_t &offset : offsets) {
            if(*reinterpret_cast<std::uint8_t *>(hook_data + offset) == 0xE8) {
                // Find where this call instruction is calling.
                auto &op = *reinterpret_cast<std::uintptr_t *>(hook_data + offset + 1);
                const auto *actual_address = (jmp_at_byte + offset + 5) + op;

                // Update the instruction
                op = reinterpret_cast<std::uintptr_t>(actual_address) - reinterpret_cast<std::uintptr_t>(hook_data + offset + 5);
            }
        }

        // Write a jmp to the original function after all is said and done
        hook_data += bytes.size();
        *reinterpret_cast<std::uint8_t *>(hook_data) = 0xE9;
        *reinterpret_cast<std::uintptr_t *>(hook_data + 1) = reinterpret_cast<const std::byte *>(jmp_at) + bytes.size() - (hook_data + 5);
    }

    void write_code(void *pointer, const SigByte *data, std::size_t length) noexcept {
        // Instantiate our new_protection and old_protection variables.
        DWORD new_protection = PAGE_EXECUTE_READWRITE, old_protection;

        // Apply read/write/execute protection
        VirtualProtect(pointer, length, new_protection, &old_protection);

        // Copy
        for(std::size_t i = 0; i < length; i++) {
            if(data[i] != -1) {
                *(reinterpret_cast<std::uint8_t *>(pointer) + i) = static_cast<std::uint8_t>(data[i]);
            }
        }

        // Restore the older protection unless it's the same
        if(new_protection != old_protection) {
            VirtualProtect(pointer, length, old_protection, &new_protection);
        }
    }
}
