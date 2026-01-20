;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _effect_shader_reindex_pixel_shader_asm
_effect_shader_reindex_pixel_shader_asm:
    pushfd
    pushad
    push ebx
    call _effect_shader_reindex_pixel_shader
    add esp, 4
    popad
    popfd
    mov eax, dword ptr [_effect_shader_permutation_index]
    jmp dword ptr [_effect_shader_get_ps_original]
