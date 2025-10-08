;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.global _skip_accessing_sound_pas_data_asm
_skip_accessing_sound_pas_data_asm:
    mov esi, dword ptr [edi + 0x220]
    cmp esi, 0
    je dont_crash_pls
    jmp dword ptr [_original_cluster_sound_distances_inst]

    dont_crash_pls:
    xor al, al
    pop esi
    ret