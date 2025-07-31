;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.global _schi_set_sampler_states_for_af_asm
_schi_set_sampler_states_for_af_asm:
    push esi
    push edi
    call _set_sampler_states_for_chicago
    add esp, 8
    ret

.global _scex_set_sampler_states_for_af_asm
_scex_set_sampler_states_for_af_asm:
    push esi
    push ebp
    call _set_sampler_states_for_chicago
    add esp, 8
    ret