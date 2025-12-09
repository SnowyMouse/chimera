;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _hud_scale_bitmap_bounds_w
_hud_scale_bitmap_bounds_w:
    pushfd
    pushad
    push dword ptr [_use_highres_scale_is_set]
    push esi
    call _check_half_scale_bitmap_flag
    add esp, 8
    popad
    popfd
    mov ebp, dword ptr [_scaled_bound_w]
    jmp dword ptr [_original_get_hud_bounds_w]

;# This must always be run after hud_scale_bitmap_bounds_w

.global _hud_scale_bitmap_bounds_h
_hud_scale_bitmap_bounds_h:
    mov ecx, dword ptr [_scaled_bound_h]
    mov dword ptr [_use_highres_scale_is_set], 0
    jmp dword ptr [_original_get_hud_bounds_h]
    
.global _mask_scale_bitmap_bounds
_mask_scale_bitmap_bounds:
    pushfd
    pushad
    push 0
    push edx
    call _check_half_scale_bitmap_flag
    add esp, 8
    popad
    popfd
    mov eax, dword ptr [_scaled_bound_w]
    mov dword ptr [esp + 0x10], eax
    mov eax, dword ptr [_scaled_bound_h]
    fild dword ptr [esp + 0x10]
    jmp dword ptr [_original_get_mask_bounds]

.global _marker_scale_bitmap_bounds
_marker_scale_bitmap_bounds:
    pushfd
    pushad
    push 0
    push ebp
    call _check_half_scale_bitmap_flag
    add esp, 8
    popad
    popfd
    mov ecx, dword ptr [_scaled_bound_w]
    mov edx, dword ptr [_scaled_reg_pt_w]
    mov [esp + 0x10], ecx
    mov ecx, dword ptr [_scaled_bound_h]
    fild dword ptr [esp + 0x10]
    mov [esp + 0x10], edx
    mov edx, dword ptr [_scaled_reg_pt_h]
    jmp dword ptr [_original_get_marker_bounds]

.global _nav_number_offset_bitmap_bounds_w
_nav_number_offset_bitmap_bounds_w:
    pushfd
    pushad
    push 0
    push ebx
    call _check_half_scale_bitmap_flag
    add esp, 8
    popad
    popfd
    mov ecx, dword ptr [_scaled_bound_w]
    jmp dword ptr [_original_get_nav_bounds_w]

;# This must always be run after nav_number_offset_bitmap_bounds_w

.global _nav_number_offset_bitmap_bounds_h
_nav_number_offset_bitmap_bounds_h:
    mov eax, dword ptr [_scaled_bound_h]
    jmp dword ptr [_original_get_nav_bounds_h]

.global _hud_static_element_child_anchor_asm
_hud_static_element_child_anchor_asm:
    lea eax, [esp + 0x2C]
    mov ebx, [esp + 0x30]

    ;# bit dodgy but should always be valid
    sub ebx, 0x24

    push ebx
    push eax
    call _hud_static_element_use_child_anchors
    add esp, 8
    ret

.global _hud_meter_element_child_anchor_asm
_hud_meter_element_child_anchor_asm:
    lea eax, [esp + 0x28]

    ;# bit dodgy but should always be valid
    sub esi, 0x24

    push esi
    push eax
    call _hud_meter_element_use_child_anchors
    add esp, 8
    ret

.global _hud_number_element_child_anchor_asm
_hud_number_element_child_anchor_asm:
    lea eax, [esp + 0x2C]
    mov ebx, [esp + 0x30]

    ;# bit dodgy but should always be valid
    sub ebx, 0x24

    push ebx
    push eax
    call _hud_number_element_use_child_anchors
    add esp, 8
    ret

.global _hud_overlay_element_child_anchor_asm
_hud_overlay_element_child_anchor_asm:
    lea eax, [esp + 0x28]
    mov ebx, [esp + 0x2C]

    ;# bit dodgy but should always be valid
    sub ebx, 0x24

    push ebx
    push eax
    call _hud_overlay_element_use_child_anchors
    add esp, 8
    ret
