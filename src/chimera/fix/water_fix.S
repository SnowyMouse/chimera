;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _water_opacity_psh_const_asm
_water_opacity_psh_const_asm:
    push 0x2
    push esi
    call _set_water_shader_const
    add esp, 8
    ret

.global _water_reflection_psh_const_asm
_water_reflection_psh_const_asm:
    push 0x1
    push esi
    call _set_water_shader_const
    add esp, 8
    ret

.global _water_bumpmap_memes_asm
_water_bumpmap_memes_asm:
    push [esp + 0x3C]
    call _make_water_go_brr
    add esp, 4
    ret

.global _water_bumpmap_memes_2_asm
_water_bumpmap_memes_2_asm:
    push ecx
    call _make_water_go_brr_pt2
    add esp, 4
    ret

.global _adjust_water_render_target_mip_asm
_adjust_water_render_target_mip_asm:
    mov eax, dword ptr [_render_target_create_loop_counter]
    inc eax
    mov dword ptr [_render_target_create_loop_counter], eax
    cmp eax, 0x8
    je set_mip_levels
    ret

    set_mip_levels:
    mov dword ptr [esp + 0x34], 0x4
    mov dword ptr [_render_target_create_loop_counter], 0
    ret