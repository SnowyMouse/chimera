;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.global _overwrite_stock_fx_data_asm
_overwrite_stock_fx_data_asm:
    ;# Check device caps for ps_2_0 support
    pushfd
    pushad
    call _effect_load_check_device_caps
    cmp al, 1
    je use_stock_fx

    ;# Replace the pointer to the loaded shaders to the chimera internal one
    popad
    popfd
    mov eax, dword ptr [_patched_shader_collection_ptr]
    mov [esp], eax
    mov eax, dword ptr [_patched_d3dx_effects_size]
    mov [esp + 4], eax
    jmp dword ptr [_original_effect_load_instruction]

    use_stock_fx:
    popad
    popfd
    jmp dword ptr [_original_effect_load_instruction]


.global _get_effect_collection_ptr_asm
_get_effect_collection_ptr_asm:
    mov dword ptr [_default_shader_collection_ptr], eax
    ret

.global _overwrite_stock_effect_collection_data_asm
_overwrite_stock_effect_collection_data_asm:
    ;# Check device caps for ps_2_0 support
    call _effect_load_check_device_caps
    cmp al, 1
    je use_stock_collection

    ;# Replace the pointer to the loaded shaders to the chimera internal one
    mov eax, dword ptr [_default_shader_collection_ptr]
    mov ebx, dword ptr [_patched_shader_collection_ptr]
    mov [eax], ebx

    use_stock_collection:
    ret


.global _overwrite_stock_vsh_data_asm
_overwrite_stock_vsh_data_asm:
    ;# Replace the pointer to the loaded shaders to the chimera internal one
    mov eax, dword ptr [_patched_vsh_collection_ptr]
    mov [esp], eax
    mov eax, dword ptr [_patched_vsh_collection_size]
    mov [esp + 4], eax
    jmp dword ptr [_original_vsh_load_instruction]

    use_stock_vsh:
    jmp dword ptr [_original_vsh_load_instruction]
