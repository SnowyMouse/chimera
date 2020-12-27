;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _extend_limits_cmp_1
_extend_limits_cmp_1:
    push eax
    mov eax, [_extend_limits_polies_count]
    mov eax, [eax]
    cmp eax, [_extend_limits_poly_limit]
    pop eax
    jmp dword ptr [_extend_limits_count_1]

.globl _extend_limits_cmp_2
_extend_limits_cmp_2:
    push eax
    mov eax, [_extend_limits_polies_count]
    mov eax, [eax]
    cmp eax, [_extend_limits_poly_limit]
    pop eax
    jmp dword ptr [_extend_limits_count_2]

.globl _extend_limits_cmp_3
_extend_limits_cmp_3:
    push eax
    mov eax, [_extend_limits_polies_count]
    mov eax, [eax]
    cmp eax, [_extend_limits_poly_limit]
    pop eax
    jmp dword ptr [_extend_limits_count_3]
