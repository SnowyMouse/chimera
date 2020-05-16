;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _execute_script_asm
_execute_script_asm:
    mov eax, [esp+4]
    pushfd
    pushad
    push eax
    call dword ptr [esp+0x30]
    pop eax
    popad
    popfd
    ret
