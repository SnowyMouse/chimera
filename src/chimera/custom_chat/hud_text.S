;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_pickup_hud_text_asm
_on_pickup_hud_text_asm:
    pushfd
    pushad
    push dword ptr [esp+0x50]
    push dword ptr [esp+0x34]
    call _on_pickup_hud_text
    add esp, 0x8
    popad
    popfd
    ret

.globl _on_hud_text_esi_asm
_on_hud_text_esi_asm:
    pushfd
    pushad
    push esi
    push dword ptr [esp+0x34]
    call _on_hold_hud_text
    add esp, 0x8
    popad
    popfd
    ret

.globl _on_weapon_pick_up_hud_text_asm
_on_weapon_pick_up_hud_text_asm:
    pushfd
    pushad
    push dword ptr [esp+0x48]
    push dword ptr [esp+0x34]
    call _on_weapon_pick_up_hud_text
    add esp, 0x8
    popad
    popfd
    ret

.globl _on_names_above_heads_hud_text_asm
_on_names_above_heads_hud_text_asm:
    pushfd
    pushad
    push ecx
    push dword ptr [esp+0x34]
    call _on_names_above_heads_hud_text
    add esp, 0x8
    popad
    popfd
    ret

zero: .long 0

.globl _hud_text_fmul_with_0_asm
_hud_text_fmul_with_0_asm:
    fmul dword ptr [zero]
    fadd dword ptr [_hud_text_new_line_spacing]
    ret
