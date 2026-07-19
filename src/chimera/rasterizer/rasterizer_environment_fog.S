;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.global _screen_fog_draw
_screen_fog_draw:
    push [esp + 24]
    push [esp + 24]
    push [esp + 24]
    push [esp + 24]
    call _rasterizer_environment_fog_screen_draw
    add esp, 16
    ret

.global _rasterizer_fog_screen_model_begin_asm
_rasterizer_fog_screen_model_begin_asm:
    push esi
    call _rasterizer_model_begin_hook
    add esp, 4
    ret

.global _rasterizer_fog_screen_model_submit_asm
_rasterizer_fog_screen_model_submit_asm:
    cmp byte ptr [_local_environment_fog_screen_flag], 0
    je return
    push [esp + 0x40]
    push [esp + 0x40]
    push [esp + 0x40]
    push [esp + 0x40]
    push [esp + 0x40]
    push [esp + 0x40]
    push [esp + 0x40]
    call _rasterizer_environment_fog_screen_model_submit
    add esp, 28

    return:
    ret