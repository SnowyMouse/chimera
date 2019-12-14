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
