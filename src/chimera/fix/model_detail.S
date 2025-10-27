;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _model_detail_fix
_model_detail_fix:
    ;# Just the bit we nopped out to get correct flow control
    mov [ebp - 8], edx
    lea ecx, [ecx]

    ;# Check for cryopod. Always scale LOD by 480p for it.
    push eax
    mov eax, dword ptr [ebx + 4]
    cmp eax, 0x0206A16D
    pop eax
    je set_scaled_lod_bias

    ;# Check for forced max lod. Otherwise use default resolution dependent scale
    cmp byte ptr [_force_max_lod_level], 0
    je default_scaling

    jmp dword ptr [_model_detail_set_lod_skip]

    set_scaled_lod_bias:
    call _set_model_detail_scale
    fld dword ptr [ebp+0x08]
    fdiv dword ptr [_lod_fix_scale]
    fstp dword ptr [ebp+0x08]

    default_scaling:
    jmp dword ptr [_model_detail_fix_original_instr]
