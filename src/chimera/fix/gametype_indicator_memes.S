;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _hud_render_gametype_draw_this_frame_asm
_hud_render_gametype_draw_this_frame_asm:
    pushfd
    pushad
    call _hud_render_gametype_draw_this_frame
    popad
    popfd
    cmp byte ptr [_hud_render_draw_gametype], 0
    je skip_gametype_draw
    jmp dword ptr [_original_gametype_draw_instr]

    skip_gametype_draw:
    jmp dword ptr [_skip_gametype_draw_ptr]

.global _hud_render_get_tag_for_ctf
_hud_render_get_tag_for_ctf:
    mov eax, dword ptr [_tag_id_for_ctf]
    ret

.global _hud_render_get_tag_for_slayer
_hud_render_get_tag_for_slayer:
    mov eax, dword ptr [_tag_id_for_slayer]
    ret

.global _hud_render_get_tag_for_oddball
_hud_render_get_tag_for_oddball:
    mov eax, dword ptr [_tag_id_for_oddball]
    ret

.global _hud_render_get_tag_for_king
_hud_render_get_tag_for_king:
    mov eax, dword ptr [_tag_id_for_king]
    ret

.global _hud_render_get_tag_for_race
_hud_render_get_tag_for_race:
    mov eax, dword ptr [_tag_id_for_race]
    ret

.global _hud_render_get_tag_for_background
_hud_render_get_tag_for_background:
    mov eax, dword ptr [_tag_id_for_bg]
    ret
