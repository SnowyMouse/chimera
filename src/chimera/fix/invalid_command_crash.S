;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
meme_global_text: .string "rasterizer_soft_filter"
ignore_broken_globals: .long 0

.text

.globl _handle_invalid_global_crash_asm
_handle_invalid_global_crash_asm:
    ;# Ignore?
    cmp dword ptr [ignore_broken_globals], 1
    je done_handle_invalid_global_crash_asm

    ;# Did we get 0xFFFF?
    cmp ax, 0xFFFF

    ;# If not, keep going
    jne done_handle_invalid_global_crash_asm

    ;# Otherwise, try again, but this time put "rasterizer_soft_filter" into ebx as this effectively does nothing
    lea ebx, [meme_global_text]
    call dword ptr [_halo_get_global_index_fn]

    ;# Done!
    done_handle_invalid_global_crash_asm:
    ret

.globl _should_ignore_broken_globals_asm
_should_ignore_broken_globals_asm:
    mov dword ptr [ignore_broken_globals], 1
    ret

.globl _should_not_ignore_broken_globals_asm
_should_not_ignore_broken_globals_asm:
    mov dword ptr [ignore_broken_globals], 0
    ret
