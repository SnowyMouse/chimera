;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
load_main_menu_addr: .long 0

.text
load_main_menu_sig: .string "load_main_menu_sig"

.globl _load_main_menu_asm
_load_main_menu_asm:
    pushfd
    pushad
    cmp dword ptr load_main_menu_addr, 0
    jnz load_main_menu
    lea eax, byte ptr load_main_menu_sig
    push eax
    call _address_for_signature
    mov load_main_menu_addr, eax
    pop eax

    load_main_menu:
    call dword ptr load_main_menu_addr ;# function address by Sledmine
    popad
    popfd
