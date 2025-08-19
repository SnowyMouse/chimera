;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _water_opacity_psh_const_asm
_water_opacity_psh_const_asm:
    push 0x2
    push esi
    call _set_water_shader_const
    add esp, 8
    ret

.global _water_reflection_psh_const_asm
_water_reflection_psh_const_asm:
    push 0x1
    push esi
    call _set_water_shader_const
    add esp, 8
    ret