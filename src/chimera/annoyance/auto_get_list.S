;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
getting_from_internet: .long 0

.text

.globl _auto_get_list_thing_asm
_auto_get_list_thing_asm:
    ;# This will be 1 if we're on the INTERNET page. We need this for later
    mov byte ptr [getting_from_internet], al
    mov al, 0
    ret

.globl _auto_get_list_thing_add_asm
_auto_get_list_thing_add_asm:
    ;# Check if we're on internet. If this is zero, then don't do anything to esi. Otherwise, set esi (the time to next query) to -1
    cmp byte ptr [getting_from_internet], 0
    je done_getting_list_automatically

    ;# Set to -1 (never refresh)
    or esi, -1

    done_getting_list_automatically:
    ret
