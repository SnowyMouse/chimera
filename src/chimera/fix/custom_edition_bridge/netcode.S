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

;# Override name packet by injecting the CRC32 into it
;# eax = pointer to packet being sent to server (unencrypted)
.globl _override_join_name_packet_asm
_override_join_name_packet_asm:
    ;# Set this stuff here
    mov dword ptr [eax + 0x93], 0x0220980E
    mov word ptr [eax], 0x94C

    ;# Override this meme stuff
    push ebx
    lea ebx, [eax + 0x8F]
    mov [esp], ebx
    mov ebx, [edi + 0xC]
    push ebx
    call _calculate_crc32_of_current_map_file_please
    add esp, 4
    pop ebx
    ret

;# Override name packet size
;# edi = 0x10 times the size of the packet + 0xC
.globl _override_join_name_packet_size_asm
_override_join_name_packet_size_asm:
    add edi, 0x1040
    ret
