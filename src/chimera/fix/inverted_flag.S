;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _invert_fog_atmospheric_dominant_flag_asm
_invert_fog_atmospheric_dominant_flag_asm:
    pushad
    pushfd
    test dx, 1          ;# check water flag
    jnz done_inverting  ;# if water flag is non-zero, do NOT flip the atmospheric dominant flag (doing this is probably wrong, but it WILL look worse)
    xor dx, 2           ;# otherwise flip the atmospheric dominant flag
    mov [esp+0x18], edx
    done_inverting:
    popfd
    popad
    ret
