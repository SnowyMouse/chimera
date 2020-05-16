;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
    .globl _new_contrail_update_function
    _new_contrail_update_function:
        ;# Check if we can update
        push eax
        mov eax, dword ptr [_can_update_contrail]
        test al, al

        ;# If nonzero, we can update
        jnz do_the_thing
        pop eax

        ;# Nope? Okay
        ret

        ;# Let's do this!
        do_the_thing:

        ;# First, move zero to our _can_update_contrail value
        xor eax, eax
        mov dword ptr [_can_update_contrail], eax

        ;# Next, set value in the stack to 1 / effective tick rate
        mov eax, dword ptr [_update_contrail_by]
        mov [esp + 0x8], eax

        ;# We're done. Update
        pop eax
        jmp dword ptr [_original_contrail_update_function]
