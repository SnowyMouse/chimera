;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
widescreen_changed_left: .short 0
widescreen_changed_right: .short 0

.text

;# Here is the widescreen_element_reposition_letterbox function.
.globl _widescreen_element_reposition_letterbox
_widescreen_element_reposition_letterbox:

push eax
mov eax, [esp + 4]
pushfd
pushad
push eax
call _reposition_letterbox_element
add esp, 4
popad
popfd
pop eax

jmp dword ptr _widescreen_element_position_letterbox_fn

.globl _widescreen_cutscene_text_before_asm
_widescreen_cutscene_text_before_asm:
    ;# Check if right-aligned
    push eax
    mov ax, word ptr [ebx+0xC]
    cmp ax, 1
    jne finish_widescreen_cutscene_text

    ;# Add the offset to the left
    mov ax, word ptr [ebx+0x6]
    cmp ax, 0
    je skip_changing_right
    mov word ptr [widescreen_changed_left], ax
    add ax, word ptr [_widescreen_left_offset_add]
    add ax, word ptr [_widescreen_left_offset_add]
    mov word ptr [ebx+0x6], ax

    skip_changing_right:

    ;# Add the offset to the right
    mov ax, word ptr [ebx+0x2]
    cmp ax, 0
    je finish_widescreen_cutscene_text
    mov word ptr [widescreen_changed_right], ax
    add ax, word ptr [_widescreen_left_offset_add]
    add ax, word ptr [_widescreen_left_offset_add]
    mov word ptr [ebx+0x2], ax

    ;# Done
    jmp finish_widescreen_cutscene_text

.globl _widescreen_cutscene_text_after_asm
_widescreen_cutscene_text_after_asm:
    ;# Check if right-aligned
    push eax
    mov ax, word ptr [ebx+0xC]
    cmp ax, 1
    jne finish_widescreen_cutscene_text

    ;# Restore the original values
    mov ax, word ptr [widescreen_changed_left]
    mov word ptr [ebx+0x6], ax
    mov ax, word ptr [widescreen_changed_right]
    mov word ptr [ebx+0x2], ax

    ;# Done
    jmp finish_widescreen_cutscene_text

finish_widescreen_cutscene_text:
    pop eax
    ret
