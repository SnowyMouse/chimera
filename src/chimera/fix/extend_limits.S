.intel_syntax noprefix

.text

.globl _extend_limits_cmp_1
_extend_limits_cmp_1:
    push eax
    mov eax, [_extend_limits_polies_count]
    mov eax, [eax]
    cmp eax, [_extend_limits_poly_limit]
    pop eax
    jmp dword ptr [_extend_limits_count_1]

.globl _extend_limits_cmp_2
_extend_limits_cmp_2:
    push eax
    mov eax, [_extend_limits_polies_count]
    mov eax, [eax]
    cmp eax, [_extend_limits_poly_limit]
    pop eax
    jmp dword ptr [_extend_limits_count_2]

.globl _extend_limits_cmp_3
_extend_limits_cmp_3:
    push eax
    mov eax, [_extend_limits_polies_count]
    mov eax, [eax]
    cmp eax, [_extend_limits_poly_limit]
    pop eax
    jmp dword ptr [_extend_limits_count_3]

.global _bsp_render_calls
_bsp_render_calls:
    ;#call _bsp_poly_begin_loop
    ;#mov ebx, 0
    call dword ptr [_bsp_render_1]
    call dword ptr [_bsp_render_2]
    ;#call dword ptr [_bsp_render_3]
    ;#call _bsp_poly_continue
    ;#cmp eax, 1
    ;#je _bsp_render_calls
    jmp dword ptr [_bsp_render_finally]
