.intel_syntax noprefix

.text

.globl _map_loading_asm
_map_loading_asm:
    push dword ptr [esp+0x38]
    push dword ptr [esp+0x2C]
    call _do_map_loading_handling
    add esp, 8
    ret
