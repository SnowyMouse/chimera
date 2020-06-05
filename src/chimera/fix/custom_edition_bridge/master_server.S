;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _override_master_server_list_asm
_override_master_server_list_asm:
    lea eax, [eax + 0x54]
    push eax
    call _do_override_master_server
    add esp, 4
    ret
