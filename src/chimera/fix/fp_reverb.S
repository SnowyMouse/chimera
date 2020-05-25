;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _override_non_firing_fp_reverb_asm
_override_non_firing_fp_reverb_asm:
    cmp ax, 4 ;# is it a firing sound?
    je ignore_fp_firing_reverb
    mov ax, 7 ;# if not, then tell lies on the internet
    ignore_fp_firing_reverb:
    ret

.globl _meme_up_reverb_position_asm
_meme_up_reverb_position_asm:
    push eax
    pushfd
    pushad
    push eax
    push ebx
    call _have_fun_with_positioning
    add esp, 8
    popad
    popfd
    pop eax
    ret
