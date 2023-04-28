;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
create_object_query_fn: .long 0
create_object_fn: .long 0
delete_object_fn: .long 0

.text
create_object_query_fn_sig: .string "create_object_query_sig"
create_object_fn_sig: .string "create_object_sig"
delete_object_fn_sig: .string "delete_object_sig"

.globl _create_object_query_asm
_create_object_query_asm:
    cmp dword ptr create_object_query_fn, 0
    jnz do_create_query

    lea eax, byte ptr create_object_query_fn_sig
    push eax
    call _address_for_signature
    add esp, 4
    sub eax, 6
    mov create_object_query_fn, eax

    do_create_query:

    ;# Push parent object ID
    mov eax, dword ptr [esp + 0x8]
    push eax

    ;# Push object tag id
    mov eax, dword ptr [esp + 0x8]
    push eax

    ;# Move query buffer to eax
    mov eax, dword ptr [esp + 0x14]

    call dword ptr create_object_query_fn
    add esp, 8

    ret

.globl _create_object_asm
_create_object_asm:
    cmp dword ptr create_object_fn, 0
    jnz do_create_object

    lea eax, byte ptr create_object_fn_sig
    push eax
    call _address_for_signature
    add esp, 4
    sub eax, 24
    mov create_object_fn, eax

    do_create_object:

    ;# Push object type
    mov eax, dword ptr [esp + 0x8]
    push eax

    ;# Push query
    mov eax, dword ptr [esp + 0x8]
    push eax

    call dword ptr create_object_fn
    add esp, 8

    ret

.globl _delete_object_asm
_delete_object_asm:
    cmp dword ptr delete_object_fn, 0
    jnz do_delete_object
    lea eax, byte ptr delete_object_fn_sig
    push eax
    call _address_for_signature
    add esp, 4
    sub eax, 10
    mov delete_object_fn, eax

    do_delete_object:
    mov eax, dword ptr [esp + 0x4]
    push eax
    call dword ptr delete_object_fn
    add esp, 4

    ret
