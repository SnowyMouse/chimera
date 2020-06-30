;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
should_override_memes: .long 0

.text

.globl _set_decal_memery_asm
_set_decal_memery_asm:
    ;# We're going to be doing fun comparison stuff. Better hold the flags here
    pushfd

    ;# Don't do anything if we're NOT double multiply
    cmp ax, 2
    jne finish_set_decal_memery

    ;# Check if we're on the alpha tested layer
    push eax
    mov eax, [ecx + edx + 0x14]
    mov ax, word ptr [eax + 0x4]
    cmp ax, 3
    jne do_nothing_to_fix_decal_memery

    ;# Override us with the regular multiply call thing
    mov dword ptr [should_override_memes], 1

    ;# Finish up here
    do_nothing_to_fix_decal_memery:
    pop eax

    ;# Done!
    finish_set_decal_memery:
    popfd
    ret

.globl _fix_decal_memery_asm
_fix_decal_memery_asm:
    pushfd
    cmp dword ptr [should_override_memes], 1
    jne finish_fix_decal_memery
    mov ax, 1
    mov dword ptr [should_override_memes], 0

    finish_fix_decal_memery:
    popfd
    ret
