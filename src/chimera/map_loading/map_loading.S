.intel_syntax noprefix

.text

.globl _free_map_handle_bugfix_asm
_free_map_handle_bugfix_asm:
    push esi
    call _do_free_map_handle_bugfix
    add esp, 4
    ret

;# fix a resource file leak (mainly with the trial version of the game)
.globl _map_loading_asm
_map_loading_asm:
    push dword ptr [esp+0x38]
    push dword ptr [esp+0x2C]
    call _do_map_loading_handling
    add esp, 8
    ret

.globl _on_read_map_file_data_asm
_on_read_map_file_data_asm:
    push esi
    push ebx
    push ebp
    push eax
    call _on_read_map_file_data
    add esp, 0x10
    cmp eax, 0
    je _on_read_map_file_data_asm_end
    mov dword ptr [esp+0x14], 0 ;# set the size to 0 if the function returns 1. yes this is janky and I'm proud
    _on_read_map_file_data_asm_end:
    ret
