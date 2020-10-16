;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
load_ui_map_addr: .long 0

.text
load_ui_map_sig: .string "load_ui_map_sig"

.globl _load_ui_map_asm
_load_ui_map_asm:
    pushfd
    pushad
    cmp dword ptr load_ui_map_addr, 0
    jnz load_ui_map
    lea eax, byte ptr load_ui_map_sig
    push eax
    call _address_for_signature
    mov load_ui_map_addr, eax
    pop eax

    load_ui_map:
    mov ebx, 0
    push ebx
    call dword ptr load_ui_map_addr
    add esp, 4
    popad
    popfd

    ret
