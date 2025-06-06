;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _frustum_hack_asm
_frustum_hack_asm:
    push ebx
    call _meme_the_transparent_decals
    add esp, 4
    ret

.global _frustum_reset_asm
_frustum_reset_asm:
    cmp byte ptr [_reset_frustum], 0
    je skip_hack_reset
    pushfd
    pushad
    push 0
    push 0
    call dword ptr [_rasterizer_set_frustum_z]
    add esp, 8
    mov byte ptr [_reset_frustum], 0
    popad
    popfd

    skip_hack_reset:
    cmp byte ptr [ebx], 0
    jns skip_fruc_reset
    jmp dword ptr [_original_get_zbias]

    skip_fruc_reset:
    jmp dword ptr [_reset_zbias]