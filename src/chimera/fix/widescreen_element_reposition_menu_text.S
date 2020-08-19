;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text


.globl _widescreen_element_reposition_menu_text
_widescreen_element_reposition_menu_text:

pushfd
pushad
push ecx
call _reposition_menu_text_element
add esp, 4
popad
popfd

jmp dword ptr _widescreen_element_position_menu_text_fn


.globl _widescreen_element_reposition_menu_text_2
_widescreen_element_reposition_menu_text_2:

pushfd
pushad
push ecx
call _reposition_menu_text_element
add esp, 4
popad
popfd

jmp dword ptr _widescreen_element_position_menu_text_2_fn


.globl _widescreen_element_reposition_text_f1
_widescreen_element_reposition_text_f1:

pushfd
pushad
add dword ptr [ecx], 0x40000 ;# add an extra 4 virtual pixels to account for widescreen fix memery
push ecx
call _reposition_menu_text_element
add esp, 4
popad
popfd

jmp dword ptr _widescreen_element_position_text_f1_fn


.globl _widescreen_element_reposition_text_pgcr
_widescreen_element_reposition_text_pgcr:

pushfd
pushad
push ecx
call _reposition_menu_text_element
add esp, 4
popad
popfd

jmp dword ptr _widescreen_element_position_text_pgcr_fn


.globl _widescreen_element_reposition_text_stare_name
_widescreen_element_reposition_text_stare_name:

pushfd
pushad
push ecx
call _reposition_menu_text_element
add esp, 4
popad
popfd

jmp dword ptr _widescreen_element_position_text_stare_name_fn


.globl _widescreen_element_reposition_text_f3_name
_widescreen_element_reposition_text_f3_name:

pushfd
pushad
push ecx
call _upscale_menu_text_element
add esp, 4
popad
popfd

jmp dword ptr _widescreen_element_position_text_f3_name_fn



.globl _widescreen_input_text
_widescreen_input_text:

pushfd
pushad
mov dword ptr _widescreen_text_input_element, ecx
call _reposition_menu_text_input
popad
popfd

ret


.globl _widescreen_input_text_undo
_widescreen_input_text_undo:

pushfd
pushad
call _unreposition_menu_text_input
popad
popfd

ret
