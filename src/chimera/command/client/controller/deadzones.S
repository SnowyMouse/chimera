;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.globl _handle_deadzones_asm
_handle_deadzones_asm:
    pushad
    pushfd
    push [esp+0x3C]
    call _handle_deadzones_cpp
    add esp, 4
    mov word ptr [esp+0x3C], ax
    popfd
    popad
    ret
