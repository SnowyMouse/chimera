;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Basically just run the meme code to write fixed positions per frame
.global _camera_shake_fix_asm
_camera_shake_fix_asm:
    pushfd
    pushad
    add eax, 40
    push eax
    sub eax, 36
    push eax
    call _meme_up_camera_shake
    add esp, 8
    popad
    popfd

    ret