;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Get the scoreboard font and put it into the esi register
.globl _get_scoreboard_font_esi_asm
_get_scoreboard_font_esi_asm:
    pushad
    pushfd
    call _get_scoreboard_font
    mov [esp+0x8], eax
    popfd
    popad
    ret

;# Get the scoreboard font and put it into the edx register
.globl _get_scoreboard_font_edx_asm
_get_scoreboard_font_edx_asm:
    pushad
    pushfd
    call _get_scoreboard_font
    mov [esp+0x18], eax
    popfd
    popad
    ret

;# Get the name font into the eax register
.globl _get_name_font_eax_asm
_get_name_font_eax_asm:
    pushad
    pushfd
    call _get_name_font
    mov [esp+0x20], eax
    popfd
    popad
    ret

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
    push dword ptr [ecx]
    push esi
    push dword ptr [esp+0x38]
    call _on_hold_hud_text
    add esp, 0xC
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
    push 0
    push ecx
    push dword ptr [esp+0x38]
    call _on_names_above_heads_hud_text
    add esp, 0xC
    popad
    popfd
    ret

.globl _on_stare_hud_text_asm
_on_stare_hud_text_asm:
    pushfd
    pushad
    push 1
    push ecx
    push dword ptr [esp+0x38]
    call _on_names_above_heads_hud_text
    add esp, 0xC
    popad
    popfd
    ret

.globl _on_menu_hud_text_double_scaled_asm
_on_menu_hud_text_double_scaled_asm:
    pushfd
    pushad
    push 0xFFFFFFFF
    push 2
    push ecx
    push dword ptr [esp+0x3C]
    call _on_menu_hud_text
    add esp, 0x10
    popad
    popfd
    ret

.globl _on_menu_hud_text_double_scaled_large_asm
_on_menu_hud_text_double_scaled_large_asm:
    pushfd
    pushad
    push 3
    push 2
    push ecx
    push dword ptr [esp+0x3C]
    call _on_menu_hud_text
    add esp, 0x10
    popad
    popfd
    ret

.globl _on_menu_hud_text_asm
_on_menu_hud_text_asm:
    pushfd
    pushad
    push 0xFFFFFFFF
    push 1
    push ecx
    push dword ptr [esp+0x3C]
    call _on_menu_hud_text
    add esp, 0x10
    popad
    popfd
    ret

.globl _on_menu_hud_text_large_asm
_on_menu_hud_text_large_asm:
    pushfd
    pushad
    push 3
    push 1
    push ecx
    push dword ptr [esp+0x3C]
    call _on_menu_hud_text
    add esp, 0x10
    popad
    popfd
    ret

.globl _on_menu_hud_text_scoreboard_asm
_on_menu_hud_text_scoreboard_asm:
    pushfd
    pushad
    call _get_scoreboard_font_generic
    push eax
    push 1
    add dword ptr [ecx], 0x40000 ;# add an extra 4 virtual pixels to account for widescreen fix memery
    push ecx
    push dword ptr [esp+0x3C]
    call _on_menu_hud_text
    add esp, 0x10
    popad
    popfd
    ret

.globl _on_menu_hud_text_unscaled_asm
_on_menu_hud_text_unscaled_asm:
    pushfd
    pushad
    push 0xFFFFFFFF
    push 0
    push ecx
    push dword ptr [esp+0x3C]
    call _on_menu_hud_text
    add esp, 0x10
    popad
    popfd
    ret

.globl _on_menu_hud_text_unscaled_large_asm
_on_menu_hud_text_unscaled_large_asm:
    pushfd
    pushad
    push 3
    push 0
    push ecx
    push dword ptr [esp+0x3C]
    call _on_menu_hud_text
    add esp, 0x10
    popad
    popfd
    ret

zero: .long 0

.globl _hud_text_fmul_with_0_asm
_hud_text_fmul_with_0_asm:
    fmul dword ptr [zero]
    fadd dword ptr [_hud_text_new_line_spacing]
    ret
