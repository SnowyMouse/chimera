;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text
.globl _check_for_bullshit_server_data_asm
_check_for_bullshit_server_data_asm:
    pushfd
    pushad

    ;# Push a pointer to the buffer
    mov ebx, dword ptr [eax + 0x4]
    push ebx

    ;# Push a pointer to the key we're dealing with
    mov ebx, dword ptr [esp + 0x2C]
    push ebx

    ;# Hold eax in the ebx register so we can set it back
    mov ebx, eax

    ;# Call it
    call _strip_bullshit_server_data
    add esp, 0x8

    ;# Move it back
    mov dword ptr [ebx + 4], eax

    popad
    popfd
    ret
