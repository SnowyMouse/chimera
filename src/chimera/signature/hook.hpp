// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HOOK_HPP
#define CHIMERA_HOOK_HPP

#include <windows.h>
#include <memory>
#include <vector>
#include <cstddef>

#include "signature.hpp"

namespace Chimera {
    /**
     * A hook is used to execute Chimera code from Halo code. It is recommended to store these as a static variable.
     */
    class Hook {
    public:
        /** This contains the original bytes. */
        std::vector<std::byte> original_bytes;

        /** This is the address to the first modified byte in Halo's code. */
        std::byte *address;

        /** This is the code being jumped to. */
        std::unique_ptr<std::byte []> hook;

        /**
         * Roll back the hook. This will write original_bytes to address and then clear original_bytes.
         */
        void rollback() noexcept;
    };

    /**
     * Write an x86 jmp instruction over the given instruction, copying the original instruction to a hook.
     * @param jmp_at             This is a pointer to the instruction to overwrite.
     * @param hook               This is the hook to write the changes to.
     * @param call_before        This is the function to call before the original instruction is run.
     * @param call_after         This is the function to call after the original instruction is run and just before returning to Halo's code.
     * @param bool pushad_pushfd Use pushad/pushfd
     */
    void write_jmp_call(void *jmp_at, Hook &hook, const void *call_before = nullptr, const void *call_after = nullptr, bool pushad_pushfd = true);

    /**
     * Override the given function with a function.
     * @param jmp_at            This is a pointer to the function to override.
     * @param hook              This is the hook to write the changes to.
     * @param new_function      This is the function to override with.
     * @param original_function This is an address that can be called for the original function.
     */
    void write_function_override(void *jmp_at, Hook &hook, const void *new_function, const void **original_function);

    /**
     * Overwrite the data at the pointer with the given bytes, ignoring any wildcard bytes.
     * @param pointer This is the pointer that points to the data to be overwritten.
     * @param data    This is the pointer that points to the data to be copied, excluding wildcards.
     * @param length  This is the length of the data.
     */
    void write_code(void *pointer, const SigByte *data, std::size_t length) noexcept;

    /**
     * Overwrite the data at the pointer with the given SigByte bytes. Bytes equal to -1 are ignored.
     * @param  pointer pointer to the data
     * @param  data    bytes to overwrite with
     */
    #define write_code_s(pointer, data) \
        static_assert(sizeof(data[0]) == sizeof(SigByte), "write_code_s requires a SigByte");\
        for(std::size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++) {\
            if(data[i] == -1) continue;\
            overwrite(pointer + i, static_cast<std::uint8_t>(data[i]));\
        }

    /**
     * Overwrite the data at the pointer with the given data even if this pointer is read-only.
     * @param pointer This is the pointer that points to the data to be overwritten.
     * @param data    This is the pointer that points to the data to be copied.
     * @param length  This is the length of the data.
     */
    template<typename T> inline void overwrite(void *pointer, const T *data, std::size_t length) noexcept {
        // Instantiate our new_protection and old_protection variables.
        DWORD new_protection = PAGE_EXECUTE_READWRITE, old_protection;

        // Apply read/write/execute protection
        VirtualProtect(pointer, length, new_protection, &old_protection);

        // Copy
        std::copy(data, data + length, reinterpret_cast<T *>(pointer));

        // Restore the older protection unless it's the same
        if(new_protection != old_protection) {
            VirtualProtect(pointer, length, old_protection, &new_protection);
        }
    }

    /**
     * Overwrite the data at the pointer with the given data even if this pointer is read-only.
     * @param pointer This is the pointer that points to the data to be overwritten.
     * @param data    This is the pointer that points to the data to be copied.
     */
    template<typename T> inline void overwrite(void *pointer, const T &data) noexcept {
        return overwrite(pointer, &data, 1);
    }
}

#endif
