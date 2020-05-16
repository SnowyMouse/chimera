;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
    .globl _new_motion_sensor_update_function
    _new_motion_sensor_update_function:
        ;# Check if we can update
        push eax
        mov eax, dword ptr [_can_update_motion_sensor]
        test al, al

        ;# If nonzero, we can update
        jnz do_the_thing
        pop eax

        ;# Nope? Okay
        ret

        ;# Let's do this!
        do_the_thing:

        ;# First, move zero to our _can_update_motion_sensor value
        xor eax, eax
        mov dword ptr [_can_update_motion_sensor], eax

        ;# We're done. Update
        pop eax
        jmp dword ptr [_original_motion_sensor_update_function]
