;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
players_command_text: .string "players"

.text
.globl _handle_invalid_command_crash_asm
_handle_invalid_command_crash_asm:
    ;# Did we get 0xFFFF?
    cmp ax, 0xFFFF

    ;# If not, keep going
    jne done_handle_invalid_command_crash_asm

    ;# Otherwise, try again, but this time put "players" into edx as this effectively does nothing
    lea edx, [players_command_text]
    call dword ptr [_halo_get_function_index_fn]

    ;# Done!
    done_handle_invalid_command_crash_asm:
    ret
