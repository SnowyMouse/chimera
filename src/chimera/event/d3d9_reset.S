;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_d3d9_reset_asm
_on_d3d9_reset_asm:
    pushfd
    pushad

    push dword ptr [esp+0x28]
    push dword ptr [esp+0x30]
    call _do_d3d9_reset_event
    add esp, 8

    popad
    popfd
    ret
