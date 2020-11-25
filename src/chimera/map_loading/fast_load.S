;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Here's the the on_get_crc32_hook function. We just need to call our function, do the stuff Halo PC does and return.
.globl _on_get_crc32_hook
_on_get_crc32_hook:
    ;# Do this
    shl eax, 4

    ;# Call our function
    pushfd
    pushad
    call _on_get_crc32_custom_edition_loading
    
    ;# Move the CRC32 value
    mov dword ptr [esp + 0x18], eax
    
    popad
    popfd
    ret
