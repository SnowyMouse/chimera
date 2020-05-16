;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
chat_out_addr: .long 0

.text

;# Handle sending chat
chat_out_sig: .string "chat_out_sig"
.globl _chat_out_asm
_chat_out_asm:
    pushfd
    pushad

    ;# If we don't have the address, get it
    cmp dword ptr chat_out_addr, 0
    jnz do_send_chat
    lea eax, byte ptr chat_out_sig
    push eax
    call _address_for_signature
    mov chat_out_addr, eax
    pop eax

    ;# Prepare our message
    do_send_chat:
    mov eax, [esp+0x28]
    mov edx, [esp+0x2C]
    mov esi, [esp+0x2C]
    push [esp+0x2C]
    sub esp, 4

    ;# Get our ID, too (for trial)
    pushfd
    pushad
    call _chat_get_local_rcon_id
    mov [esp+0x24], eax
    popad
    popfd

    ;# Send the message
    call dword ptr chat_out_addr
    add esp, 8

    popad
    popfd
    ret
