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

.globl _f2_fade_fix_asm
_f2_fade_fix_asm:
    ;# Store ecx (the float) into the stack and have eax point to it
    push eax
    push ecx
    mov eax, esp

    ;# Push registers/flags to not get wrecked, passing eax (which is just a pointer) into the interpolate function, and then restoring registers/flags
    pushfd
    pushad
    push eax
    call _interpolate_f2_fade
    add esp, 4
    popad
    popfd

    ;# Pop ecx (which will now hold the new float) and eax
    pop ecx
    pop eax
    ret
