;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_windowed_check_force_windowed
_on_windowed_check_force_windowed:
    or eax, dword ptr [_force_windowed_mode]
    ret

.global _on_set_present_params_asm
_on_set_present_params_asm:
    push ebp
    call _override_d3d_present_parameters
    add esp, 4
    ret
