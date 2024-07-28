;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
init_value: .long 0x3D088889

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

.globl _interpolation_memes
_interpolation_memes:
    ;# Check if interpolation is enabled
    push eax
    mov eax, dword ptr [_apply_interpolation_hack]
    test al, al
    pop eax

    ;# If nonzero, apply the hack
    jnz hacky_memes

    ;# Interpolation disabled? Proceed as normal
    jmp dword ptr [_original_instruction]

    hacky_memes:
    
    ;# Block contrail update if first tick after creation.
    push eax
    mov eax, dword ptr [init_value]
    cmp eax, dword ptr [esi + 0x24]
    pop eax
    jg skip
    jmp dword ptr [_original_instruction]

    skip:
    jmp dword ptr [_skip_update]