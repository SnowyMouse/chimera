;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_set_video_mode_initially
_on_set_video_mode_initially:
    push [esp+0x30]
    call _set_vsync_setting_initially
    add esp, 4
    ret

.globl _on_windowed_check_force_windowed
_on_windowed_check_force_windowed:
    or eax, dword ptr [_force_windowed_mode]
    test al, al
    jz done_forcing_windowed
    pushfd
    pushad
    call _now_set_borderless_windowed_mode
    popad
    popfd
    done_forcing_windowed:
    ret
