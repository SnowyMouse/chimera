;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
load_ui_widget_addr: .long 0
load_main_menu_addr: .long 0

.text
load_ui_widget_sig: .string "load_ui_widget_sig"
load_main_menu_sig: .string "load_main_menu_sig"
load_main_menu_demo_sig: .string "load_main_menu_demo_sig"

.globl _load_ui_widget_asm
_load_ui_widget_asm:
    pushfd
    pushad
    cmp dword ptr load_ui_widget_addr, 0
    jnz load_ui_widget
    lea eax, byte ptr load_ui_widget_sig
    push eax
    call _address_for_signature
    mov load_ui_widget_addr, eax
    pop eax

    load_ui_widget:
    mov eax, [esp+0x28]
    push -1
    push -1
    push -1
    push 0
    push 0
    push -1
    push eax
    call dword ptr load_ui_widget_addr ;# function address by Sledmine.
    pop eax
    add esp, 24
    popad
    popfd

    ret

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
    call dword ptr load_main_menu_addr
    popad
    popfd

    ret
