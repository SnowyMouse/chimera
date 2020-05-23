;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _delete_object_asm
_delete_object_asm:
    push eax
    mov eax, dword ptr [esp+0x8]

    pushfd
    pushad
    push eax
    call dword ptr [_delete_object_fn]
    add esp, 4
    popad
    popfd

    pop eax
    ret
