.intel_syntax noprefix

.text

.globl _on_read_map_data
_on_read_map_data:
    pushfd
    pushad
    push [esp+0x30]
    lea eax, [esp+0x30]
    push eax
    push [esp+0x30]
    push [esp+0x30]
    call _override_file_read
    add esp, 16
    popad
    popfd
    jmp dword ptr [_on_read_map_data_actually_do_it_now]

.globl _on_select_map
_on_select_map:
    push [esp+0x28]
    call _handle_select_map
    add esp, 4
    ret
