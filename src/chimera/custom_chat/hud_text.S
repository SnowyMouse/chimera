;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_pickup_hud_text_asm
_on_pickup_hud_text_asm:
    push ebx
    mov ebx, esp
    pushfd
    pushad
    push dword ptr [esp+0x54]
    push dword ptr [esp+0x38]
    call _on_pickup_hud_text
    add esp, 0x8
    mov dword ptr [ebx], eax
    popad
    popfd
    pop ebx
    ret
