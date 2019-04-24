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
