;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _hud_meter_use_pixel_shader_asm
_hud_meter_use_pixel_shader_asm:
    pushfd
    pushad
    mov eax, dword ptr [_hud_meter_draw_vertices_offset]
    mov esi, dword ptr [esp + eax]
    push esi
    push edi
    call _hud_meter_shader_draw
    add esp, 8
    cmp al, 0
    je use_stock_rendering

    popad
    popfd
    jmp dword ptr [_skip_ff_hud_meters_pointer]

    use_stock_rendering:
    popad
    popfd
    jmp dword ptr [_hud_meter_use_pixel_shader_ins]


.global _hud_meter_set_xbox_channel_order_asm
_hud_meter_set_xbox_channel_order_asm:
    lea eax, [esp + 0x90]
    push esi
    push eax
    call _hud_meter_set_xbox_channel_order
    add esp, 8
    ret