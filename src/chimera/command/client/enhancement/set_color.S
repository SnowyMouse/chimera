;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _get_set_color_setting_asm
_get_set_color_setting_asm:
    ;# Set ecx to a pointer to its old value
    push ecx
    mov ecx, esp

    ;# pushad/fd this stuff so we don't mangle everything
    pushfd
    pushad

    ;# Get the color setting, backing up ecx (I think srand() mangles it?)
    push ecx
    call _get_set_color_setting
    pop ecx
    mov dword ptr [ecx], eax

    ;# Done
    popad
    popfd

    ;# Now set ecx to the thing
    pop ecx

    ;# Done!
    ret
