;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
should_overwrite_volume_this_time: .long 0

.text

.globl _override_ting_volume_set_asm
_override_ting_volume_set_asm:
    ;# See if we can override the ting volume. Reserve some space on the stack to hold the stuff, since we're going to use pushfd/pushad to guard against the stack being squished
    push ebx
    sub esp, 4
    mov ebx, esp
    pushfd
    pushad
    call _using_custom_map_on_retail
    movsx eax, al ;# Promote to a 32-bit int
    mov dword ptr [ebx], eax ;# Store result here
    popad
    popfd

    ;# Unreserve the space and move it into ebx
    pop ebx

    ;# If that's zero, we can't mess with the ting volume. Otherwise, set it!
    test ebx, ebx
    jz finish_override_ting_volume_set_asm
    mov dword ptr [should_overwrite_volume_this_time], 1

    finish_override_ting_volume_set_asm:
    pop ebx
    ret

.globl _override_ting_volume_write_asm
_override_ting_volume_write_asm:
    ;# Did we set the should_overwrite_volume_this_time flag earlier? If not, return.
    cmp dword ptr [should_overwrite_volume_this_time], 1
    jne finish_override_ting_volume_write_asm

    ;# If so, set the new ting and clear the flag
    mov dword ptr [esp+0x28], 0x3E4CCCCD ;# 0.2
    mov dword ptr [should_overwrite_volume_this_time], 0

    finish_override_ting_volume_write_asm:
    ret
