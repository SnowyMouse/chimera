;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _multitexture_check_for_map_ce_asm
_multitexture_check_for_map_ce_asm:
    lea edx, [esp + 0x120]
    push edx
    push esi
    call _adjust_pixel_shader_const
    add esp, 8
    ret

.globl _multitexture_check_for_map_retail_asm
_multitexture_check_for_map_retail_asm:
    lea edx, [esp + 0x124]
    push edx
    push esi
    call _adjust_pixel_shader_const
    add esp, 8
    ret

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