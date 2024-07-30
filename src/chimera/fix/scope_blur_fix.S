;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _radius_fix_asm
_radius_fix_asm:
    fld dword ptr [esp + 0x38]
    fmul dword ptr [_blur_fix_scale]
    fstp dword ptr [esp + 0x38]
    ret
