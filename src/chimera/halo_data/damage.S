;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
apply_damage_addr: .long 0

.text

;# Damage signature
apply_damage_sig: .string "apply_damage_sig"

.globl _apply_damage_asm
_apply_damage_asm:
    pushfd
    pushad

    ;# If we don't have the signature, get it
    lea eax, dword ptr apply_damage_addr
    push eax
    lea eax, byte ptr apply_damage_sig
    push eax
    call _address_for_signature_if_null
    add esp, 8

    do_apply_damage:

    ;# Pass some stuff
    push 0
    push 0xFFFFFFFF
    push 0xFFFFFFFF
    push 0xFFFFFFFF

    ;# Object ID
    push dword ptr [esp+0x3C]

    ;# Struct pointer
    push dword ptr [esp+0x3C]

    ;# Call it and hope we don't crash
    call dword ptr apply_damage_addr

    ;# Cleanup
    add esp, 0x18

    popad
    popfd

    ret
