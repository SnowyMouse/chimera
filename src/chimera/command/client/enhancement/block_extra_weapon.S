;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _block_extra_weapon_asm
_block_extra_weapon_asm:
    pushfd
    pushad
    ;# Promote ax to a 32-bit integer and push it
    movsx eax, ax
    push eax

    ;# Call our thing
    call _get_next_actual_unblocked_weapon
    add esp, 4

    ;# Check if we got 0xFFFFFFFF. If not, set ax to it
    cmp eax, 0xFFFFFFFF
    je finish_blocking_extra_weapon
    mov dword ptr [esp+0x1C], eax

    ;# Done
    finish_blocking_extra_weapon:
    popad
    popfd
    ret
