.intel_syntax noprefix

.text

;# Get the scoreboard font and put it into the esi register
.globl _get_scoreboard_font_esi_asm
_get_scoreboard_font_esi_asm:
    pushad
    pushfd
    call _get_scoreboard_font
    mov [esp+0x8], eax
    popfd
    popad
    ret

;# Get the scoreboard font and put it into the edx register
.globl _get_scoreboard_font_edx_asm
_get_scoreboard_font_edx_asm:
    pushad
    pushfd
    call _get_scoreboard_font
    mov [esp+0x18], eax
    popfd
    popad
    ret
