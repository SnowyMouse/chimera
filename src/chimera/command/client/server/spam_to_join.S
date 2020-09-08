;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.globl _on_error_dialog_asm
_on_error_dialog_asm:
    ;# check if it's index 0x2E which is "The server is full."
    cmp si, 0x2E
    jne skip_spam_to_join
    call _on_spam_to_join
    skip_spam_to_join:
    ret
