;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _model_detail_fix
_model_detail_fix:
    fld dword ptr [ebp+0x08]
    fdiv dword ptr [_lod_fix_scale]
    fstp dword ptr [ebp+0x08]
    ret
