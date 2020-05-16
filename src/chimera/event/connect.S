;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_preconnect_asm
_on_preconnect_asm:
    pushfd
    pushad

    lea ecx, dword ptr [ecx+0x2]
    push ecx
    lea eax, dword ptr [ebx+0x12]
    push eax
    push ebx
    call _on_preconnect
    add esp, 0xC
    cmp eax, 0
    je block_event

    popad
    popfd
    jmp dword ptr [_continue_preconnect]

    block_event:
    popad
    popfd
    xor eax, eax
    ret
