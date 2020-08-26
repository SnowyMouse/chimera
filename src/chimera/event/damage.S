;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_damage_asm
_on_damage_asm:
    pushfd
    pushad

    lea eax, dword ptr [esp+0x28]
    push eax
    lea eax, dword ptr [esp+0x28]
    push eax
    call _do_damage_event
    add esp, 8

    test al,al
    jnz do_damage_effect
    popad
    popfd
    ret

    do_damage_effect:
    popad
    popfd
    jmp dword ptr [_do_continue_damage_effect]
