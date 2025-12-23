;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _frustum_hack_asm
_frustum_hack_asm:
    pushfd
    pushad
    push eax
    push ebx
    call _meme_the_transparent_decals
    add esp, 8
    popad
    popfd
    mov al, byte ptr [_force_zbias_hack]
    ret

.global _frustum_reset_asm
_frustum_reset_asm:
    pushfd
    pushad
    call _reset_frustum_hacks
    popad
    popfd
    jmp dword ptr [_original_get_zbias]