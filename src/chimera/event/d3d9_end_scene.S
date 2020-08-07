;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_d3d9_end_scene_asm
_on_d3d9_end_scene_asm:
    pushfd
    pushad

    push dword ptr [esp+0x28]
    call _do_d3d9_end_scene_event
    add esp, 4

    popad
    popfd
    ret
