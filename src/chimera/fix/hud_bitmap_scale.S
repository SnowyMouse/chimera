;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _hud_scale_bitmap_bounds_w
_hud_scale_bitmap_bounds_w:
    pushfd
    pushad
    push esi
    call _check_half_scale_bitmap_flag
    add esp, 4
    popad
    popfd
    mov ebp, dword ptr [_scaled_bound_w]
    jmp dword ptr [_original_get_hud_bounds_w]

.global _hud_scale_bitmap_bounds_h
_hud_scale_bitmap_bounds_h:
    pushfd
    pushad
    push esi
    call _check_half_scale_bitmap_flag
    add esp, 4
    popad
    popfd
    mov ecx, dword ptr [_scaled_bound_h]
    mov dword ptr [_use_highres_scale_is_set], 0
    jmp dword ptr [_original_get_hud_bounds_h]
    
.global _mask_scale_bitmap_bounds
_mask_scale_bitmap_bounds:
    mov dword ptr [_use_highres_scale_is_set], 0
    pushfd
    pushad
    push edx
    call _check_half_scale_bitmap_flag
    add esp, 4
    popad
    popfd
    mov eax, dword ptr [_scaled_bound_w]
    mov dword ptr [esp + 0x10], eax
    mov eax, dword ptr [_scaled_bound_h]
    fild dword ptr [esp + 0x10]
    jmp dword ptr [_original_get_mask_bounds]

.global _marker_scale_bitmap_bounds
_marker_scale_bitmap_bounds:
    mov dword ptr [_use_highres_scale_is_set], 0
    pushfd
    pushad
    push ebp
    call _check_half_scale_bitmap_flag
    add esp, 4
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
