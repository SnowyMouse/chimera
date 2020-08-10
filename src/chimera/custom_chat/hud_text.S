;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_pickup_hud_text_asm
_on_pickup_hud_text_asm:
    push ebx
    mov ebx, esp
    pushfd
    pushad
    push dword ptr [esp+0x54]
    push dword ptr [esp+0x38]
    call _on_pickup_hud_text
    add esp, 0x8
    mov dword ptr [ebx], eax
    popad
    popfd
    pop ebx
    ret

.globl _on_hold_to_pick_up_hud_text_asm
_on_hold_to_pick_up_hud_text_asm:
    pushfd
    pushad
    push dword ptr [esp+0x68]
    push esi
    push dword ptr [esp+0x38]
    call _on_hold_hud_text
    add esp, 0xC
    popad
    popfd
    ret

.globl _on_hold_to_pick_up_hud_text_button_asm
_on_hold_to_pick_up_hud_text_button_asm:
    pushfd
    pushad
    push 0x12345678
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
    lea eax, dword ptr [esp+0x48]
    push eax
    push dword ptr [esp+0x34]
    call _on_weapon_pick_up_hud_text
    add esp, 0x8
    popad
    popfd
    ret
