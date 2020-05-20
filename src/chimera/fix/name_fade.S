;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _name_fade_fix_dec_asm
_name_fade_fix_dec_asm:
    ;# Check if we can change the fade
    push eax
    mov eax, dword ptr [_can_do_name_fade_dec]
    test eax, eax
    pop eax

    ;# If not, don't do anything
    jz skip_name_fade_fix

    ;# Otherwise, apply the fade
    mov [esi+0x80], ecx

    ;# Reset the value
    mov dword ptr [_can_do_name_fade_dec], 0

    ;# Done
    ret

.globl _name_fade_fix_inc_asm
_name_fade_fix_inc_asm:
    ;# Check if we can change the fade
    push eax
    mov eax, dword ptr [_can_do_name_fade_inc]
    test eax, eax
    pop eax

    ;# If not, don't do anything
    jz skip_name_fade_fix

    ;# Otherwise, apply the fade
    mov [esi+0x80], eax

    ;# Reset the value
    mov dword ptr [_can_do_name_fade_inc], 0

    ;# Done
    ret

    ;# Skip
skip_name_fade_fix:
    ret
