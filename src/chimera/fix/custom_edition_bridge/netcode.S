;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Override master server list
;# eax        = pointer to packet being sent to master server
;# eax + 0x54 = "halor" (the type of list to use)
.globl _override_master_server_list_asm
_override_master_server_list_asm:
    test byte ptr [_custom_edition_netcode_is_enabled], 1
    jz do_not_switch_the_server_list ;# if zero (false), roll it back

    ;# If it's 1 (true), switch the list
    mov byte ptr [eax + 0x58], 'm' ;# Change 'r' to 'm' in halor
    jmp done_switching_server_list

    ;# Otherwise, don't
    do_not_switch_the_server_list:
    mov byte ptr [eax + 0x58], 'r' ;# Change 'm' to 'r' in halom

    done_switching_server_list:
    ret
