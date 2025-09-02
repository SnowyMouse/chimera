;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _set_psh_const_for_xbox_order_asm
_set_psh_const_for_xbox_order_asm:
    push edi
    call _set_psh_const_for_xbox_order
    add esp, 4
    ret
