;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _scoreboard_fade_fix_asm
_scoreboard_fade_fix_asm:
    ;# Make room to hold a float and store&pop the float here
    sub esp, 4
    fstp dword ptr [esp]

    ;# eax will hold a pointer to that float so we can pass it
    push eax
    lea eax, [esp + 4]

    ;# push registers and flags so they don't get wrecked
    pushfd
    pushad

    ;# Pass the pointer to the float we made earlier
    push eax
    call _interpolate_scoreboard_fade
    add esp, 4

    ;# restore registers and flags
    popad
    popfd

    ;# Load the float
    fld dword ptr [eax]

    ;# Restore eax and delete the float from the stack
    pop eax
    add esp, 4

    ;# Done
    ret
