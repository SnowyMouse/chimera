;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _tab_out_video_reacquire_device_subroutine_asm
_tab_out_video_reacquire_device_subroutine_asm:
    mov eax, dword ptr [_tab_out_video_disable_video_ptr]
    mov al, byte ptr [eax]
    test al, al
    jz _tab_out_video_reacquire_device_subroutine_original_asm
    ;# Video is disabled, return early with AL = 0, do not invoke ResetVideoDevice
    xor al, al
    ;# Since the hook is partially into the function, we need to properly clean up after ourselves.
    pop ebx
    add esp, 0x38
    ret
    
.globl _tab_out_video_reacquire_device_subroutine_original_asm
_tab_out_video_reacquire_device_subroutine_original_asm:
    ;# Video is not disabled, undo the hook and proceed as normal
    jmp dword ptr [_tab_out_video_device_subroutine_original]
