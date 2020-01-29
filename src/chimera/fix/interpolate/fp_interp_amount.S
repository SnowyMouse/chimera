.intel_syntax noprefix

.text
    .globl _on_fp_interp_amount
    _on_fp_interp_amount:
        push eax
        mov eax, [esp+0xC]
        mov dword ptr [_fp_interp_amount], eax
        mov eax, [esp+0x10]
        mov dword ptr [_fp_interp_amount+4], eax
        mov eax, [esp+0x14]
        mov dword ptr [_fp_interp_amount+8], eax
        pop eax
        ret
