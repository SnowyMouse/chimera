;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
chat_out_addr: .long 0

.text

;# Handle chat messages
.globl _on_chat_message
_on_chat_message:
    push [esp+0x34]
    push [esp+0x3C]
    lea eax, [ebp-0x200]
    push eax
    call _draw_chat_message
    add esp, 12
    ret

;# Handle chat open (T key)
.globl _on_chat_button
_on_chat_button:
    pushfd
    pushad
    push [esp+0x28]
    call _bring_up_chat_prompt
    add esp, 4
    popad
    popfd
    ret

;# Handle sending chat
chat_out_sig: .string "chat_out_sig"
.globl _chat_out_asm
_chat_out_asm:
    pushfd
    pushad

    cmp dword ptr chat_out_addr, 0
    jnz do_send_chat
    lea eax, byte ptr chat_out_sig
    push eax
    call _address_for_signature
    mov chat_out_addr, eax
    pop eax

    do_send_chat:
    mov eax, [esp+0x28]
    mov edx, [esp+0x2C]
    mov esi, [esp+0x2C]
    push [esp+0x2C]
    sub esp, 4

    pushfd
    pushad
    call _chat_get_local_rcon_id
    mov [esp+0x24], eax
    popad
    popfd

    call dword ptr chat_out_addr
    add esp, 8

    popad
    popfd
    ret

;# Handle kill feed
.globl _on_kill_feed
_on_kill_feed:
    pushfd
    pushad

    ;# Check if this is a welcome message
    mov eax, [esp+0x2C]
    cmp eax, 0
    jnz do_not_handle_message

    ;# It's a welcome message!
    push [esp+0x28]
    call _welcome_message
    add esp, 4
    popad
    popfd
    ret

    ;# It's not; Handle it normally
    do_not_handle_message:
    popad
    popfd
    jmp dword ptr _kill_feed_message
