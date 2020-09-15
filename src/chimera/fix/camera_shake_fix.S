;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Basically store the pointer
.globl _camera_shake_fix_asm
_camera_shake_fix_asm:
    pushfd
    pushad
    add edx, 4
    push edx
    call _meme_up_camera_shake_thing
    add esp, 4
    popad
    popfd

    push eax
    lea eax, [ebx+0xE2]
    mov dword ptr [_camera_shake_ptr], eax
    pop eax
    xor cx, cx
    ret


;# Then, every tick, subtract it by 1
.globl _camera_shake_tick_asm
_camera_shake_tick_asm:
    push eax
    mov eax, dword ptr [_camera_shake_ptr]
    test eax, eax
    jz skip
    sub word ptr [eax], 1
    mov dword ptr [_camera_shake_ptr], 0
    skip:
    pop eax
    ret
