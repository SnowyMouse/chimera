.intel_syntax noprefix

.text

;# Here's the the on_get_crc32_hook function. We just need to call our function, do the stuff Halo PC does and return.
.globl _on_get_crc32_hook
_on_get_crc32_hook:
    ;# Call our function
    pushad
    call _on_get_crc32
    popad

    ;# Do what Halo normally does at this moment
    shl eax, 4
    mov ecx, [eax + edx + 12]
    ret
