;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.globl _auto_uncrouch_asm
_auto_uncrouch_asm:
    cmp ecx, 0xA
    jne FINISH
    pushad
    pushfd
    push ecx
    call _auto_uncrouch_cpp
    pop ecx
    cmp eax, 1
    jne FINISH_CLEANUP
    ;# If we're supposed to automatically uncrouch, increment ecx by 1 to skip it
    inc ecx
    mov dword ptr [esp+0x40], ecx
    FINISH_CLEANUP:
    popfd
    popad
    FINISH:
    ret
