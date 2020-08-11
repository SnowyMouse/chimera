;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Here is the display_text() function.
;#
;# These are the parameters:
;#   - [esp + 4]:  This is a pointer to the string
;#   - [esp + 8]:  This is the x1 and y1 values in the format 0xXXXXYYYY
;#   - [esp + 12]: This is the x2 and y2 values in the format 0xXXXXYYYY. These are typically 640x480 unless the text is aligned.
;#   - [esp + 16]: This is the function to use
;#

.globl _display_text
_display_text:
    ;# Push all registers so we don't mangle them
    pushad

    ;# Allocate a struct. This will be the position where our text will be rendered.
    sub esp, 8

    ;# Copy our XXXXYYYY value
    mov ecx, [esp + 0x30]
    mov [esp], ecx

    ;# Copy our WWWWHHHH value
    mov ecx, [esp + 0x34]
    mov [esp + 4], ecx

    ;# Set ecx to our struct
    mov ecx, esp

    ;# Set eax to 0
    xor eax, eax

    ;# Push our string and a couple 0s (I don't know what the 0s are for but do it)
    push [esp + 0x2C]
    push 0
    push 0
    call dword ptr [esp + 0x44]
    add esp, 20

    ;# Pop all registers, restoring them
    popad

    ;# Done!
    ret
