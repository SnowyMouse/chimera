;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _override_fp_reverb_position_asm
_override_fp_reverb_position_asm:
    pushfd
    pushad
    push eax ;# Address of sound position we're writing to
    push ebx ;# Object ID of the related object
    call _move_current_fp_sound_to_camera
    add esp, 8
    popad
    popfd
    ret
