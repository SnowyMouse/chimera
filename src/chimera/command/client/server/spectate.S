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
