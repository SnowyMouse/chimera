;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _override_non_firing_fp_reverb_asm
_override_non_firing_fp_reverb_asm:
    cmp ax, 4 ;# is it a firing sound?
    je ignore_fp_firing_reverb
    mov ax, 0xD ;# if not, then tell lies on the internet
    ignore_fp_firing_reverb:
    ret
