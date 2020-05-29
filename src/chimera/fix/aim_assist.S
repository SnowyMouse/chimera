;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _on_aim_assist
_on_aim_assist:
    mov eax, [_using_analog_movement]
    cmp byte ptr [eax], 1
    jnz not_actually_using_aim_assist
    jmp dword ptr [_yes_using_analog_movement_jmp]
    not_actually_using_aim_assist:
    jmp dword ptr [_not_using_analog_movement_jmp]
