;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Here is the widescreen_element_reposition_menu function.
.globl _widescreen_element_reposition_menu
_widescreen_element_reposition_menu:

push eax
mov eax, [esp + 4]
pushfd
pushad
push eax
call _reposition_menu_element
add esp, 4
popad
popfd
pop eax

jmp dword ptr _widescreen_element_position_menu_fn
