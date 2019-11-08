.intel_syntax noprefix

.text

.globl _free_map_handle_bugfix_asm
_free_map_handle_bugfix_asm:
    push esi
    call _do_free_map_handle_bugfix
    add esp, 4
    ret

.globl _map_loading_asm
_map_loading_asm:
    push dword ptr [esp+0x38]
    push dword ptr [esp+0x2C]
    call _do_map_loading_handling
    add esp, 8
    ret
