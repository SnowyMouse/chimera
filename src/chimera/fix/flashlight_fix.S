;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
two_float: .long 0x3FC00000

.text

.globl _on_flashlight_asm
_on_flashlight_asm:
    test dword ptr [edi], 0x10
    jz done_flashlight_asm
    fmul dword ptr [two_float]
    done_flashlight_asm:
    ret
