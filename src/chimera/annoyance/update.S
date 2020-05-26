;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _block_update_check_asm
_block_update_check_asm:
    push eax
    mov eax, dword ptr [_update_check_already_checked]
    mov byte ptr [eax], 0
    pop eax
    mov eax, 2
    ret
