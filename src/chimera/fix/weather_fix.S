;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _new_weather_update_function
_new_weather_update_function:
    ;# Check if we can update
    push eax
    mov eax, dword ptr [_can_update_weather]
    test al, al

    ;# If nonzero, we can update
    jnz update_weather

    ;# Otherwise, do nothing
    pop eax
    add esp, 20
    ret

    update_weather:

    ;# reset _can_update_weather value
    xor eax, eax
    mov dword ptr [_can_update_weather], eax

    ;# Update
    pop eax
    jmp dword ptr [_original_call]