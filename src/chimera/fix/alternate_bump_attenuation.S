;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _set_psh_constant_for_alternate_bump_retail_asm
_set_psh_constant_for_alternate_bump_retail_asm:
    push edi
    push ebx
    call _set_psh_constant_for_alternate_bump
    add esp, 8
    ret

.global _set_psh_constant_for_alternate_bump_custom_asm
_set_psh_constant_for_alternate_bump_custom_asm:
    push edx
    push esi
    call _set_psh_constant_for_alternate_bump
    add esp, 8
    ret