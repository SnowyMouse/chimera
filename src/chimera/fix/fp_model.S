;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _fp_model_lens_flare_asm
_fp_model_lens_flare_asm:
    push esi
    call _fp_model_lens_flare_adjust_frustum
    add esp, 4
    ret
