;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.global _shader_transparent_generic_switch_function
_shader_transparent_generic_switch_function:
    mov edx, dword ptr [ebp + 0xC]
    push edx
    push ebx
    call _rasterizer_shader_transparent_generic_draw
    add esp, 0x8
    jmp dword ptr [_generic_switch_return]
