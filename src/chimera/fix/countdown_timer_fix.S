;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _initialize_hud_timer_numbers_asm
_initialize_hud_timer_numbers_asm:
    lea eax, [esp + 108]
    xor bl, bl

    init_struct_loop:
    mov dword ptr [eax], 0
    add eax, 4
    inc bl
    cmp bl, 21
    jl init_struct_loop

    ret
