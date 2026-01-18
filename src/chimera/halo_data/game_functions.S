;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _rasterizer_transparent_geometry_group_draw
_rasterizer_transparent_geometry_group_draw:
    push [esp + 8]
    push [esp + 8]
    call dword ptr [_rasterizer_transparent_geometry_group_draw_func]
    add esp, 8
    ret

.globl _shader_get_vertex_shader_permutation
_shader_get_vertex_shader_permutation:
    ;# param in ECX
    push ecx
    mov ecx, dword ptr [esp + 8]
    call dword ptr [_shader_get_vertex_shader_permutation_func]
    pop ecx
    ret

.globl _rasterizer_set_framebuffer_blend_function
_rasterizer_set_framebuffer_blend_function:
    ;# param in ECX
    push ecx
    mov ecx, dword ptr [esp + 8]
    call dword ptr [_rasterizer_set_framebuffer_blend_function_func]
    pop ecx
    ret

.globl _rasterizer_set_texture
_rasterizer_set_texture:
    push eax
    mov eax, dword ptr [esp + 20 + 4]
    push [esp + 16 + 4]
    push [esp + 16 + 4]
    push [esp + 16 + 4]
    push [esp + 16 + 4]
    call dword ptr [_rasterizer_set_texture_func]
    add esp, 16
    pop eax
    ret

.globl _shader_texture_animation_evaluate
_shader_texture_animation_evaluate:
    push edi
    push ebx
    push ecx
    push esi
    mov edi, [esp + 40 + 16]
    mov ebx, [esp + 36 + 16]
    mov ecx, [esp + 32 + 16]
    mov esi, [esp + 28 + 16]
    push [esp + 24 + 16]
    push [esp + 24 + 16]
    push [esp + 24 + 16]
    push [esp + 24 + 16]
    push [esp + 24 + 16]
    push [esp + 24 + 16]
    call dword ptr [_shader_texture_animation_evaluate_func]
    add esp, 24
    pop esi
    pop ecx
    pop ebx
    pop edi
    ret

.globl _rasterizer_transparent_geometry_group_draw_vertices
_rasterizer_transparent_geometry_group_draw_vertices:
    push ecx
    mov ecx, dword ptr [esp + 8]
    push [esp + 12]
    call dword ptr [_rasterizer_transparent_geometry_group_draw_vertices_func]
    add esp, 4
    pop ecx
    ret

.globl _periodic_function_evaluate
_periodic_function_evaluate:
    push eax
    mov eax, [esp + 16]
    push [esp + 12]
    push [esp + 8]
    call dword ptr [_periodic_function_evaluate_func]
    add esp, 8
    pop eax
    ret

.globl _rasterizer_set_frustum_z
_rasterizer_set_frustum_z:
    push [esp + 8]
    push [esp + 8]
    call dword ptr [_rasterizer_set_frustum_z_func]
    add esp, 8
    ret

.globl _rasterizer_draw_dynamic_triangles_static_vertices2
_rasterizer_draw_dynamic_triangles_static_vertices2:
    push eax
    push edi
    mov edi, dword ptr [esp + 24]
    mov eax, dword ptr [esp + 20]
    push [esp + 28]
    push [esp + 20]
    push [esp + 20]
    call dword ptr [_rasterizer_draw_dynamic_triangles_static_vertices2_func]
    add esp, 12
    pop edi
    pop eax
    ret

.globl _render_camera_build_frustum
_render_camera_build_frustum:
    push eax
    push ecx
    push esi
    push [esp + 28]
    mov esi, [esp + 28]
    mov eax, [esp + 24]
    mov ecx, [esp + 20]
    call dword ptr [_render_camera_build_frustum_func]
    add esp, 4
    pop esi
    pop ecx
    pop eax
    ret
