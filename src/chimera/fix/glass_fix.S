;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.global _set_correct_glass_diffuse_psh_asm
_set_correct_glass_diffuse_psh_asm:
    mov eax, dword ptr [edi + 0x5C]
    test eax, eax
    jz use_diffuse_model

    ;# Go to 2nd permutation
    add edx, 0x00000084

    use_diffuse_model:
    jmp dword ptr [_original_set_glass_diffuse_instr]

.global _set_glass_reflection_ps_const_custom_asm
_set_glass_reflection_ps_const_custom_asm:
    push ecx
    push edi
    call _set_correct_glass_reflection_ps_const
    add esp, 8
    ret

.global _set_glass_reflection_ps_const_retail_asm
_set_glass_reflection_ps_const_retail_asm:
    ;# move c_group_intensity.x into c_eye_forward.w
    lea ecx, [esp + 0x88]
    mov ebx, [esp + 0xAC]
    mov [ecx], ebx
    ret
