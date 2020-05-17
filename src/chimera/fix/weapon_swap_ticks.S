;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _weapon_swap_ticks_fix_asm
_weapon_swap_ticks_fix_asm:
    pushfd
    pushad

    ;# Check if a tick has passed
    mov ebx, dword ptr [_can_increment_weapon_swap_tick_counter]
    test ebx, ebx
    jz load_weapon_slot_tick_counter

    ;# Since a tick has passed, reset the check
    mov dword ptr [_can_increment_weapon_swap_tick_counter], 0

    ;# Now... is the frame counter from Halo currently nonzero?
    test eax, eax
    jz reset_incrementing_weapon_swap_tick

    ;# If nonzero (thus the button is held down), increment
    mov eax, dword ptr [_weapon_swap_tick_counter]
    inc eax
    mov dword ptr [_weapon_swap_tick_counter], eax
    jmp load_weapon_slot_tick_counter

    ;# If zero (button is not held down), reset to 0
    reset_incrementing_weapon_swap_tick:
    xor eax, eax
    mov dword ptr [_weapon_swap_tick_counter], eax

    ;# Done
    load_weapon_slot_tick_counter:
    mov eax, dword ptr [_weapon_swap_tick_counter]

    ;# Overwrite eax with our new eax value
    mov [esp+0x1C], eax

    popad
    popfd
    ret
