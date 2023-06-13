;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Basically tie the save loop to the tick counter
.globl _checkpoint_fix_asm
_checkpoint_fix_asm:
    ;# Check if game is trying to force an unsafe save. If so, don't interfere
    push eax
    mov eax, dword ptr [_safe_save]
    cmp byte ptr [eax], 0
    pop eax
    je unsafe_save

    ;# Otherwise, check if a tick event has occured this frame, if not, skip the save function this frame
    cmp dword ptr [_ticks_passed], 0
    je skip_save_func

    ;# If one or more tick events have occured this frame, run the save function for as many ticks have passed this frame.
    try_to_save:
    push eax
    mov eax, dword ptr [_ticks_passed]
    dec eax
    mov dword ptr [_ticks_passed], eax
    pop eax
    call dword ptr [_save_func]
    cmp dword ptr [_ticks_passed], 0
    jg try_to_save

    skip_save_func:
    jmp dword ptr [_skip_save_func]

    ;# reset counters and run save loop
    unsafe_save:
    mov dword ptr [_ticks_passed], 0
    jmp dword ptr [_original_save_call]
