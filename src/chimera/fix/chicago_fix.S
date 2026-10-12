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

.global _chicago_set_up_pixel_shader_asm
_chicago_set_up_pixel_shader_asm:
    call _rasterizer_shader_transparent_chicago_draw
    cmp al, 1
    je skip_ff_draw
    jmp dword ptr[_chicago_draw_original_ptr]

    skip_ff_draw:
    mov eax, dword ptr[_chicago_draw_original_ptr]
    add eax, 5
    jmp eax

.global _chicago_extended_set_up_pixel_shader_asm
_chicago_extended_set_up_pixel_shader_asm:
    call _rasterizer_shader_transparent_chicago_extended_draw
    cmp al, 1
    je skip_ex_ff_draw
    jmp dword ptr[_chicago_extended_draw_original_ptr]

    skip_ex_ff_draw:
    mov eax, dword ptr[_chicago_extended_draw_original_ptr]
    add eax, 5
    jmp eax
