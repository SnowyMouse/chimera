;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
should_overwrite_volume_this_time: .long 0
close_server_connection_address: .long 0

.text
close_server_connection_sig: .string "close_server_connection_sig"

.globl _free_map_handle_bugfix_asm
_free_map_handle_bugfix_asm:
    push esi
    call _do_free_map_handle_bugfix
    add esp, 4
    ret

.globl _data_map_loading_asm
_data_map_loading_asm:
    push dword ptr [esp+0x30]
    push dword ptr [esp+0x2C]
    call _get_data_map_path
    add esp, 8
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

.globl _override_ting_volume_set_asm
_override_ting_volume_set_asm:
    ;# See if we can override the ting volume. Reserve some space on the stack to hold the stuff, since we're going to use pushfd/pushad to guard against the stack being squished
    push ebx
    sub esp, 4
    mov ebx, esp
    pushfd
    pushad
    call _using_custom_map_on_retail
    movsx eax, al ;# Promote to a 32-bit int
    mov dword ptr [ebx], eax ;# Store result here
    popad
    popfd

    ;# Unreserve the space and move it into ebx
    pop ebx

    ;# If that's zero, we can't mess with the ting volume. Otherwise, set it!
    test ebx, ebx
    jz finish_override_ting_volume_set_asm
    mov dword ptr [should_overwrite_volume_this_time], 1

    finish_override_ting_volume_set_asm:
    pop ebx
    ret

.globl _override_ting_volume_write_asm
_override_ting_volume_write_asm:
    ;# Did we set the should_overwrite_volume_this_time flag earlier? If not, return.
    cmp dword ptr [should_overwrite_volume_this_time], 1
    jne finish_override_ting_volume_write_asm

    ;# If so, set the new ting and clear the flag
    mov dword ptr [esp+0x28], 0x3E4CCCCD ;# 0.2
    mov dword ptr [should_overwrite_volume_this_time], 0

    finish_override_ting_volume_write_asm:
    ret

.globl _close_server_connection_asm
_close_server_connection_asm:
    cmp dword ptr close_server_connection_address, 0
    jnz close_server_connection_address_found
    lea eax, byte ptr close_server_connection_sig
    push eax
    call _address_for_signature
    mov close_server_connection_address, eax
    add esp, 0x4
    close_server_connection_address_found:

    mov eax, close_server_connection_address

    ;# set some flags, this is what happens when we press the esc key
    mov ebx, dword ptr [eax + 0x2]
    mov byte ptr [ebx], 0x0
    mov ebx, dword ptr [eax + 0x9]
    mov byte ptr [ebx], 0x0
    mov ebx, dword ptr [eax + 0x10]
    mov byte ptr [ebx], 0x0

    ;# this is the one that actually makes the thing close the connection
    mov ebx, dword ptr [eax + 0x17]
    mov byte ptr [ebx], 0x1

    ;# call cleanup function (or whatever it is, I'm not sure what it does)
    mov ebx, dword ptr [eax + 0x1D]
    lea ecx, dword ptr [eax + 0x21]
    add ebx, ecx
    call ebx

    ret
