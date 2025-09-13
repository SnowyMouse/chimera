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

.global _set_correct_glass_reflection_psh_asm
_set_correct_glass_reflection_psh_asm:
    cmp word ptr [esp + 0xA4], 00
    je reflection_bumped

    mov ax, word ptr [ebx + 0x28]
    shr eax, 3
    and al, 0x1
    test al, al
    jz reflection_bumped

    ;# Go to 2nd permutation
    add ecx, 0x00000084

    reflection_bumped:
    jmp dword ptr [_original_set_glass_reflection_instr]