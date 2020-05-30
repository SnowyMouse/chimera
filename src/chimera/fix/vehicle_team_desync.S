;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _on_check_for_desynced_vehicle_entry_asm
_on_check_for_desynced_vehicle_entry_asm:
    pushfd
    pushad

    ;# Check if we're a dedicated server
    call _on_check_for_desynced_vehicle_entry
    test eax, eax
    jz check_desync_let_halo_do_its_thing

    ;# If we are, do immoral things to eax because Microsoft won't release the game's source code or make a good Halo port
    mov dword ptr [esp+0x1C], 1

    check_desync_let_halo_do_its_thing:
    popad
    popfd
    ret
