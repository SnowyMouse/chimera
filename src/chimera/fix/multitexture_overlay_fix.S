;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _multitexture_check_for_map_ce_asm
_multitexture_check_for_map_ce_asm:
    cmp bx, 0x2
    je fix_psh_params_custom
    jmp dword ptr [_original_multitexture_draw_ins]

    fix_psh_params_custom:
    pushfd
    pushad
    lea edx, [esp + 0x11c]
    push edx
    call _adjust_pixel_shader_const
    add esp, 4
    popad
    popfd
    jmp dword ptr [_original_multitexture_draw_ins]

.globl _multitexture_check_for_map_retail_asm
_multitexture_check_for_map_retail_asm:
    cmp bp, 0x2
    je fix_psh_params_retail
    jmp dword ptr [_original_multitexture_draw_ins]

    fix_psh_params_retail:
    pushfd
    pushad
    lea edx, [esp + 0x120]
    push edx
    call _adjust_pixel_shader_const
    add esp, 4
    popad
    popfd
    jmp dword ptr [_original_multitexture_draw_ins]

.global _set_multiexture_shader_index_ce_asm
_set_multiexture_shader_index_ce_asm:
    pushfd
    pushad
    push esi
    call _fix_the_multitexture_shader_indexing
    add esp, 4
    popad
    popfd
    cmp dword ptr [_multitexture_shader_index], 0
    je exit_custom
    mov ebx, dword ptr [_multitexture_shader_index]

    exit_custom:
    jmp dword ptr [_original_multitexture_index_ins]

.global _set_multiexture_shader_index_retail_asm
_set_multiexture_shader_index_retail_asm:
    pushfd
    pushad
    push esi
    call _fix_the_multitexture_shader_indexing
    add esp, 4
    popad
    popfd
    cmp dword ptr [_multitexture_shader_index], 0
    je exit_retail
    mov ebp, dword ptr [_multitexture_shader_index]

    exit_retail:
    jmp dword ptr [_original_multitexture_index_ins]