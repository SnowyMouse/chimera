;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Calculate and move the tick counter to eax
weapon_swap_ticks_fix_asm:
    push ebx

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

    pop ebx
    ret

;# The counter is in eax (retail and trial)
.globl _weapon_swap_ticks_fix_asm_eax
_weapon_swap_ticks_fix_asm_eax:
    jmp weapon_swap_ticks_fix_asm

;# The counter is in ecx (custom edition) - requires moving the return value to ecx
.globl _weapon_swap_ticks_fix_asm_ecx
_weapon_swap_ticks_fix_asm_ecx:
    push eax
    mov eax, ecx
    call weapon_swap_ticks_fix_asm
    mov ecx, eax
    pop eax
    ret
