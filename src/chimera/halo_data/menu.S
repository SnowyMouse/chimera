;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
load_main_menu_addr: .long 0

.text
load_main_menu_sig: .string "load_main_menu_sig"
load_main_menu_demo_sig: .string "load_main_menu_demo_sig"

.globl _load_main_menu_asm
_load_main_menu_asm:
    pushfd
    pushad

    ;# Check if we have the address already. If not, get it.
    cmp dword ptr load_main_menu_addr, 0
    jnz load_main_menu
    lea eax, byte ptr load_main_menu_sig
    push eax
    call _address_for_signature
    test eax, eax

    ;# Check if it was successful. If not, we need to use the demo sig.
    jnz found_load_signature

    ;# Nope. Try the demo sig.
    pop eax
    lea eax, byte ptr load_main_menu_demo_sig
    push eax
    call _address_for_signature

    ;# Found it.
    found_load_signature:
    mov load_main_menu_addr, eax
    pop eax

    load_main_menu:
    call dword ptr load_main_menu_addr ;# function address by Sledmine
    popad
    popfd
