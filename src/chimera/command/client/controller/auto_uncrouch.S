;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.globl _auto_uncrouch_asm
_auto_uncrouch_asm:
    cmp ecx, 0xA
    jne FINISH
    call _auto_uncrouch_cpp
    cmp eax, 1
    jne FINISH
    ;# If we're supposed to automatically uncrouch, increment ecx by 1 to skip it
    inc ecx
    mov dword ptr [esp+0x1C], ecx
    FINISH:
    ret
