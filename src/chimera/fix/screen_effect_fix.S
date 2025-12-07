;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _screen_effect_radius_fix_asm
_screen_effect_radius_fix_asm:
    fld dword ptr [esp + 0x38]
    fmul dword ptr [_blur_fix_scale]
    fstp dword ptr [esp + 0x38]
    ret

.globl _screen_effect_get_ps_masked_custom_asm
_screen_effect_get_ps_masked_custom_asm:
    pushfd
    pushad
    lea eax, [esp + 0x60]
    movsx edx, word ptr [esp + 0x38]
    movsx ecx, bp
    push eax
    push ecx
    push edx
    push ebx
    call _screen_effect_reindex_pixel_shaders
    add esp, 16
    popad
    popfd
    mov esi, dword ptr [_screen_effect_pixel_shader]
    ret

.globl _screen_effect_get_ps_nomask_custom_asm
_screen_effect_get_ps_nomask_custom_asm:
    pushfd
    pushad
    lea eax, [esp + 0x60]
    movsx edx, word ptr [esp + 0x38]
    movsx ecx, bp
    push eax
    push ecx
    push edx
    push ebx
    call _screen_effect_reindex_pixel_shaders
    add esp, 16
    popad
    popfd
    mov ecx, dword ptr [_screen_effect_pixel_shader]
    ret

.globl _screen_effect_get_ps_masked_retail_asm
_screen_effect_get_ps_masked_retail_asm:
    pushfd
    pushad
    lea eax, [esp + 0x64]
    movsx edx, word ptr [esp + 0x3C]
    movsx ecx, si
    push eax
    push ecx
    push edx
    push edi
    call _screen_effect_reindex_pixel_shaders
    add esp, 16
    popad
    popfd
    mov eax, dword ptr [_screen_effect_pixel_shader]
    ret

.globl _screen_effect_get_ps_nomask_retail_asm
_screen_effect_get_ps_nomask_retail_asm:
    jmp dword ptr [_screen_effect_set_ps_masked]