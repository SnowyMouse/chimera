;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.data
players_command_text: .string "players"

.text
.globl _handle_invalid_command_crash_asm
_handle_invalid_command_crash_asm:
    add esp, 4
    lea edx, [players_command_text]
    jmp dword ptr [_halo_precompile_script_fn]
