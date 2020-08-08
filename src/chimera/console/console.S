;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
console_fade_fn_addr: .long 0

.text
console_fade_fn_sig: .string "console_fade_fn_sig"

.globl _fade_out_console_asm
_fade_out_console_asm:
    pushfd
    pushad

    cmp dword ptr console_fade_fn_addr, 0
    jnz do_fade_out_console
    lea eax, byte ptr console_fade_fn_sig
    push eax
    call _address_for_signature
    mov console_fade_fn_addr, eax
    pop eax

    do_fade_out_console:
    call dword ptr console_fade_fn_addr

    popad
    popfd
    ret

.globl _override_console_output_eax_asm
_override_console_output_eax_asm:
    ;# Color
    push ebx
    mov ebx, eax
    add ebx, 0x103
    push ebx

    ;# String
    push eax

    call _on_console_output

    ;# Done!
    add esp, 8
    xor eax, eax
    pop ebx
    ret

.globl _override_console_output_edi_asm
_override_console_output_edi_asm:
    ;# Color
    push ebx
    mov ebx, edi
    add ebx, 0x103
    push ebx

    ;# String
    push edi

    call _on_console_output

    ;# Done!
    add esp, 8
    xor eax, eax
    pop ebx
    ret
