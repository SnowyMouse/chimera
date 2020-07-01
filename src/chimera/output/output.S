;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
rcon_out_addr: .long 0
console_output_addr: .long 0
hud_output_addr: .long 0
rcon_message_addr: .long 0

.text
rcon_out_sig: .string "rcon_out_sig"
console_output_sig: .string "console_out_sig"
hud_output_sig: .string "hud_message_sig"
rcon_message_sig: .string "rcon_message_sig"

;# Make sure anything with a % sign doesn't get wrecked
console_output_format: .string "%s"

.globl _send_rcon_message_asm
_send_rcon_message_asm:
    pushfd
    pushad

    ;# Check if we know the address
    cmp dword ptr rcon_out_addr, 0
    jnz do_rcon_output

    ;# Get the address if not set
    lea eax, byte ptr rcon_out_sig
    push eax
    call _address_for_signature
    mov rcon_out_addr, eax
    pop eax

    do_rcon_output:
    ;# Set the variables
    ;# eax = pointer to message
    ;# ebp = player index (also pushed!)
    mov eax, [esp+0x2C] ;# message
    mov ebp, [esp+0x28] ;# player
    push ebp
    call dword ptr rcon_out_addr
    add esp, 4

    ;# Clean up
    popad
    popfd
    ret

.globl _console_output_asm
_console_output_asm:
    pushfd
    pushad

    ;# Check if we know the address
    cmp dword ptr console_output_addr, 0
    jnz do_console_output

    ;# Get the address if not set
    lea eax, byte ptr console_output_sig
    push eax
    call _address_for_signature
    mov console_output_addr, eax
    pop eax

    do_console_output:
    ;# Set the variables
    ;# eax = pointer to color
    mov eax, [esp+0x28]
    lea edx, console_output_format
    push [esp+0x2C] ;# message
    push edx
    call dword ptr console_output_addr
    add esp, 8

    ;# Clean up
    popad
    popfd
    ret

.globl _hud_output_asm
_hud_output_asm:
    pushfd
    pushad

    ;# Check if we know the address
    cmp dword ptr hud_output_addr, 0
    jnz do_hud_output

    ;# Get the address if not set
    lea eax, byte ptr hud_output_sig
    push eax
    call _address_for_signature
    mov hud_output_addr, eax
    pop eax

    do_hud_output:
    ;# Set the variables
    ;# eax = 0. Idk what this does but it should be 0
    mov eax, 0
    push [esp+0x28] ;# message
    call dword ptr hud_output_addr
    add esp, 4
    popad
    popfd
    ret

.globl _before_rcon_message
_before_rcon_message:
    ;# Push the rcon message we've received
    push [esp+0x2C]
    call _on_rcon_message
    add esp, 4

    ;# Check if false was returned. If so, keep going
    cmp eax, 0
    jz block_rcon_message
    ret

    ;# Let's block it!
    block_rcon_message:

    ;# Get the rcon message addr
    cmp dword ptr rcon_message_addr, 0
    jnz finally_cancel_rcon_message
    lea eax, byte ptr rcon_message_sig
    push eax
    call _address_for_signature
    mov rcon_message_addr, eax
    pop eax

    ;# Find out where we're going
    finally_cancel_rcon_message:
    mov eax, dword ptr rcon_message_addr
    add eax, 5

    ;# Remember where we're going
    push eax
    add esp, 8
    popad
    popfd

    ;# Get out
    jmp [esp-0x2C]
