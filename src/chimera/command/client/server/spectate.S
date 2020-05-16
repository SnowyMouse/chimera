;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.globl _spectate_swap_eax_asm
_spectate_swap_eax_asm:
    pushfd
    pushad
    call _spectate_get_player_id
    mov [esp+0x1C], eax
    popad
    popfd
    ret

.globl _spectate_swap_eax_object_id_asm
_spectate_swap_eax_object_id_asm:
    pushfd
    pushad
    call _spectate_get_object_id
    cmp eax, 0xFFFFFFFF
    je sseax_oid_do_nothing
    mov [esp+0x1C], eax
    sseax_oid_do_nothing:
    popad
    popfd
    ret

.globl _spectate_swap_edx_object_id_asm
_spectate_swap_edx_object_id_asm:
    pushfd
    pushad
    call _spectate_get_object_id
    cmp eax, 0xFFFFFFFF
    je ssedx_oid_do_nothing
    mov [esp+0x14], eax
    ssedx_oid_do_nothing:
    popad
    popfd
    ret

.globl _spectate_swap_ecx_weapon_object_id_asm
_spectate_swap_ecx_weapon_object_id_asm:
    pushfd
    pushad
    call _spectate_get_player_weapon_id
    mov [esp+0x18], eax
    popad
    popfd
    ret

.globl _spectate_swap_ecx_asm
_spectate_swap_ecx_asm:
    pushfd
    pushad
    call _spectate_get_player_id
    mov [esp+0x18], eax
    popad
    popfd
    ret

.globl _spectate_object_addr_eax_asm
_spectate_object_addr_eax_asm:
    pushfd
    pushad
    call _spectate_get_player_object_addr
    mov [esp+0x1C], eax
    popad
    popfd

    cmp eax, 0
    jnz spectate_can_continue
    add esp, 4
    pop esi
    pop ebp
    add esp, 0x20

    spectate_can_continue:
    ret
