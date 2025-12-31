;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _specular_light_draw_set_texture_asm
_specular_light_draw_set_texture_asm:
    push eax
    mov eax, 0x00000003
    mov [esp + 8], eax
    pop eax
    ret

.globl _specular_light_adjust_ps_const_custom
_specular_light_adjust_ps_const_custom:
    push eax
    push ebx
    mov eax, dword ptr [_specular_light_is_spotlight]
    mov bl, [eax]
    cmp bl, 1
    pop ebx
    pop eax
    je skip_custom
    mov dword ptr [esp + 0x30], 0x00000000

    skip_custom:
    ret
    
.globl _specular_light_adjust_ps_const_retail
_specular_light_adjust_ps_const_retail:
    push eax
    push ebx
    mov eax, dword ptr [_specular_light_is_spotlight]
    mov bl, [eax]
    cmp bl, 1
    pop ebx
    pop eax
    je skip_retail
    mov dword ptr [esp + 0x2C], 0x00000000

    skip_retail:
    ret
    
.global _specular_lightmap_draw_set_constants_asm
_specular_lightmap_draw_set_constants_asm:
    mov eax, dword ptr [ebp]
    mov ecx, dword ptr [_specular_constants_table]
    mov ecx, [ecx + 0x18]
    mov ecx, [ecx]
    mov esi, dword ptr [esp + 0x30]
    mov edx, [eax]
    push esi
    push ecx
    push eax
    call dword ptr [edx + 0x88]
    mov eax, dword ptr [ebp]
    mov ecx, dword ptr [_specular_constants_table]
    mov ecx, [ecx + 0x18]
    mov ecx, [ecx + 0x4]
    mov esi, dword ptr [esp + 0x30]
    add esi, 0x10
    mov edx, [eax]
    push esi
    push ecx
    push eax
    call dword ptr [edx + 0x88]
    mov eax, dword ptr [ebp]
    mov ecx, dword ptr [_specular_constants_table]
    mov ecx, [ecx + 0x18]
    mov ecx, [ecx + 0x8]
    mov esi, dword ptr [esp + 0x30]
    add esi, 0x20
    mov edx, [eax]
    push esi
    push ecx
    push eax
    call dword ptr [edx + 0x88]
    mov eax, dword ptr [ebp]
    mov ecx, dword ptr [_specular_constants_table]
    mov ecx, [ecx + 0x18]
    mov ecx, [ecx + 0xC]
    mov esi, dword ptr [esp + 0x30]
    add esi, 0x30
    mov edx, [eax]
    push esi
    push ecx
    push eax
    call dword ptr [edx + 0x88]
    ret