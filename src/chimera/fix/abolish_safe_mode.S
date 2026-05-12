;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.global _set_memory_limit_for_demo_asm
_set_memory_limit_for_demo_asm:
    cmp ecx, 0x40000000
    jbe potato_pc

    mov ecx, 0x40000000
    mov dword ptr [esp + 0x88], ecx

    potato_pc:
    ret
