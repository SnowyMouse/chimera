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

.globl _map_loading_server_asm
_map_loading_server_asm:
    push dword ptr [esp+0x28]
    push dword ptr [esp+0x30]
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
    je on_read_map_file_data_asm_end
    mov dword ptr [esp+0x14], 0 ;# set the size to 0 if the function returns 1. yes this is janky and I'm proud
    on_read_map_file_data_asm_end:
    ret

.globl _on_map_load_multiplayer_asm
_on_map_load_multiplayer_asm:
    mov eax, dword ptr [esp+0x28]
    lea eax, [eax+0x14]
    push eax
    call _on_map_load_multiplayer
    add esp, 4
    cmp eax, 0
    je on_map_load_multiplayer_asm_end
    add esp, 4
    popad
    popfd
    push [_on_map_load_multiplayer_fail]
    on_map_load_multiplayer_asm_end:
    ret

.globl _on_server_join_text_asm
_on_server_join_text_asm:
    lea eax, _download_text_string
    mov [esp+0x4], eax
    ret

.globl _on_check_if_map_is_bullshit_asm
_on_check_if_map_is_bullshit_asm:
    mov eax, 1
    mov ecx, 1
    ret
