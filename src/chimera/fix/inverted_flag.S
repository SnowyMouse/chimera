.intel_syntax noprefix

.text

.globl _invert_fog_atmospheric_dominant_flag_asm
_invert_fog_atmospheric_dominant_flag_asm:
    pushad
    pushfd
    xor dx, 2 ;# flip the atmospheric dominant flag
    mov [esp+0x18], edx
    popfd
    popad
    ret
