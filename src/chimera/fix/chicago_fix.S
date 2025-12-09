;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _chicago_multiply_set_xbox_blending_asm
_chicago_multiply_set_xbox_blending_asm:
    push edi
    push ebp
    push ebx
    call _chicago_multiply_set_xbox_blending
    add esp, 12
    ret

.global _chicago_multiply2x_set_xbox_blending_asm
_chicago_multiply2x_set_xbox_blending_asm:
    push edi
    push ebp
    push ebx
    call _chicago_multiply2x_set_xbox_blending
    add esp, 12
    ret

.global _chicago_extended_multiply_set_xbox_blending_asm
_chicago_extended_multiply_set_xbox_blending_asm:
    push edi
    push ebp
    push ebx
    call _chicago_multiply_set_xbox_blending
    add esp, 12
    ret

.global _chicago_extended_multiply2x_set_xbox_blending_asm
_chicago_extended_multiply2x_set_xbox_blending_asm:
    mov esi, [esp + 0x28]
    push edi
    push ebx
    push esi
    call _chicago_multiply2x_set_xbox_blending
    add esp, 12
    ret

.global _chicago_set_projective_divide_asm
_chicago_set_projective_divide_asm:
    push ecx
    call _chicago_projective_divide_first_map
    add esp, 4
    ret