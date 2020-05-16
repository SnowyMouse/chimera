;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# edx = x, ecx = y

.globl _widescreen_mouse
    _widescreen_mouse:
    push edx

    ;# Y bounds is always 0-480
    mov edx, dword ptr [_widescreen_mouse_y]
    cmp ecx, 480
    jg y_too_high
    cmp ecx, 0
    jl y_too_low
    mov dword ptr [edx], ecx
    jmp y_done
    y_too_low:
    mov dword ptr [edx], 0
    jmp y_done
    y_too_high:
    mov dword ptr [edx], 480

    y_done:

    mov edx, dword ptr [_widescreen_mouse_x]
    push ebx
    push ecx
    mov ebx, dword ptr [_widescreen_mouse_left_bounds]
    mov ecx, dword ptr [_widescreen_mouse_right_bounds]

    cmp eax, ecx
    jg x_too_high
    cmp eax, ebx
    jl x_too_low
    mov dword ptr [edx], eax
    jmp x_done
    x_too_low:
    mov dword ptr [edx], ebx
    jmp x_done
    x_too_high:
    mov dword ptr [edx], ecx

    x_done:
    pop ecx
    pop ebx
    pop edx
    ret
