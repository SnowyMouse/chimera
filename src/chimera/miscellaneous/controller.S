;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# ebx = output buffer
;# esp+4 = button index
.globl _on_button_text_asm
_on_button_text_asm:
    ;# Hold our button index
    push ecx
    mov ecx, [esp + 0x8]

    ;# Store EAX too
    push eax

    ;# Call the bullshit
    pushfd
    pushad
    push ecx
    push ebx
    call _on_button_text
    add esp, 8
    mov [esp+0x1C], eax
    popad
    popfd

    ;# Finish up
    test al, al
    pop eax

    ;# Also pop this
    pop ecx

    ;# Done?
    jnz finish_text_override
    jmp dword ptr [_original_button_text_fn]


;# ebx = output buffer
;# esp+4 = axis index
;# esp+8 = axis direction (0=-,1=+)
.globl _on_axis_text_asm
_on_axis_text_asm:
    ;# Hold our axis index
    push ecx
    mov ecx, [esp + 0x8]

    ;# Hold our axis direction thing
    push edx
    mov edx, [esp + 0x10]

    ;# Store EAX too
    push eax

    ;# Call the bullshit
    pushfd
    pushad
    push edx
    push ecx
    push ebx
    call _on_axis_text
    add esp, 12
    mov [esp+0x1C], eax
    popad
    popfd

    ;# Finish up
    test al, al
    pop eax

    ;# Also pop this
    pop edx
    pop ecx

    ;# Done?
    jnz finish_text_override
    jmp dword ptr [_original_axis_text_fn]


;# ebx = output buffer
;# esp+4 = pov index
;# esp+8 = pov axis (0=n,1=ne,2=e,3=se,4=s,5=sw,5=w,6=nw)
.globl _on_pov_text_asm
_on_pov_text_asm:
    ;# Hold our pov index
    push ecx
    mov ecx, [esp + 0x8]

    ;# Hold our axis direction thing
    push edx
    mov edx, [esp + 0x10]

    ;# Store EAX too
    push eax

    ;# Call the bullshit
    pushfd
    pushad
    push edx
    push ecx
    push ebx
    call _on_pov_text
    add esp, 12
    mov [esp+0x1C], eax
    popad
    popfd

    ;# Finish up
    test al, al
    pop eax

    ;# Also pop this
    pop edx
    pop ecx

    ;# Done?
    jnz finish_text_override
    jmp dword ptr [_original_pov_text_fn]

finish_text_override:
    ret
