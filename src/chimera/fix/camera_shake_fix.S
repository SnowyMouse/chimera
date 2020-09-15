;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Basically store the pointer
.globl _camera_shake_fix_asm
_camera_shake_fix_asm:
    ;# Store the camera shaking counter pointer
    push eax
    lea eax, [ebx+0xE2]
    mov dword ptr [_camera_shake_counter_ptr], eax
    pop eax
    xor cx, cx

    pushfd
    pushad
    add edx, 4
    push edx
    call _meme_up_camera_shake_thing
    add esp, 4
    popad
    popfd

    ret

;# Then, every tick, subtract it by 1
.globl _camera_shake_tick_asm
_camera_shake_tick_asm:
    ;# Check if our counter is null. If so, skip subtracting and resetting things
    push eax
    mov eax, dword ptr [_camera_shake_counter_ptr]
    test eax, eax
    jz skip

    ;# Decrement the counter by 1
    sub word ptr [eax], 1

    ;# Reset these to 0 so we know that it's a new tick!
    mov dword ptr [_camera_shake_counter_ptr], 0
    mov dword ptr [_camera_shaking], 0

    skip:
    pop eax
    ret
