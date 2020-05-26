;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _set_sane_defaults_asm
_set_sane_defaults_asm:
    mov byte ptr [ebx + 0xA6F], 0 ;# no vsync
    mov byte ptr [ebx + 0xA70], 1 ;# specular
    mov byte ptr [ebx + 0xA71], 1 ;# shadows
    mov byte ptr [ebx + 0xA72], 1 ;# decals
    mov byte ptr [ebx + 0xA73], 2 ;# particles
    mov byte ptr [ebx + 0xA74], 2 ;# texture quality
    mov byte ptr [ebx + 0xB7B], 1 ;# environmental audio
    mov byte ptr [ebx + 0xB7C], 1 ;# hardware acceleration (in case we have EAX)
    mov byte ptr [ebx + 0xB7D], 2 ;# sound quality
    mov byte ptr [ebx + 0xB7F], 2 ;# sound variety
    ret
