;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _widescreen_element_reposition_hud
_widescreen_element_reposition_hud:
push eax
mov eax, [esp + 4]
pushfd
pushad
push eax
;# If this flag is set, we scale the element's position rather than treat it as a HUD element
cmp dword ptr _widescreen_upscale_nav_element, 0
je reposition
call _upscale_hud_element
jmp done
reposition:
call _reposition_hud_element
done:
add esp, 4
popad
popfd
pop eax
jmp dword ptr _widescreen_element_position_hud_fn

.globl _widescreen_element_reposition_multitexture_overlay
_widescreen_element_reposition_multitexture_overlay:
push eax
mov eax, [esp + 4]
pushfd
pushad
push eax
call _reposition_hud_element
add esp, 4
popad
popfd
pop eax
jmp dword ptr _widescreen_element_position_multitexture_overlay_fn

.globl _widescreen_element_reposition_motion_sensor
_widescreen_element_reposition_motion_sensor:
push eax
mov eax, [esp + 4]
pushfd
pushad
push eax
call _reposition_hud_element
add esp, 4
popad
popfd
pop eax
jmp dword ptr _widescreen_element_position_motion_sensor_fn

.globl _widescreen_element_upscale_hud
_widescreen_element_upscale_hud:
push eax
mov eax, [esp + 4]
pushfd
pushad
push eax
call _upscale_hud_element
add esp, 4
popad
popfd
pop eax
jmp dword ptr _widescreen_element_position_hud_2_fn

.globl _widescreen_set_upscale_flag
_widescreen_set_upscale_flag:

mov dword ptr _widescreen_upscale_nav_element, 1
ret

.globl _widescreen_unset_upscale_flag
_widescreen_unset_upscale_flag:

mov dword ptr _widescreen_upscale_nav_element, 0
ret

.globl _widescreen_set_hud_no_center_flag
_widescreen_set_hud_no_center_flag:

mov dword ptr _widescreen_hud_no_center, 1
ret

.globl _widescreen_unset_hud_no_center_flag
_widescreen_unset_hud_no_center_flag:

mov dword ptr _widescreen_hud_no_center, 0
ret
