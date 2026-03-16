;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _check_for_invalid_bitmap_format_asm
_check_for_invalid_bitmap_format_asm:
    push esi
    call _check_for_invalid_bitmap_format
    add esp, 4
    ret
