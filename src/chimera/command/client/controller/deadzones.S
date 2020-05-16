;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.globl _handle_deadzones_asm
_handle_deadzones_asm:
    push [esp+0x18]
    call _handle_deadzones_cpp
    add esp, 4
    mov word ptr [esp+0x18], ax
    ret
