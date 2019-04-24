.intel_syntax noprefix

.text

.globl _on_set_video_mode_initially
_on_set_video_mode_initially:
    push [esp+0x30]
    call _set_vsync_setting_initially
    add esp, 4
    ret
