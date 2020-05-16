;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

.globl _fun_cd_key_hash_function_asm
_fun_cd_key_hash_function_asm:
    push [esp+0x30]
    call _fun_cd_key_hash_gen_asm
    add esp, 4
    ret
