// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" {
        std::uint32_t block_update_check_asm() noexcept;
        std::uint8_t *update_check_already_checked; // did we already check for updates?
    }

    void enable_block_update_check() noexcept {
        auto &chimera = get_chimera();
        
        static Hook hook;
        auto *check_for_update_fn_call_sig = chimera.get_signature("check_for_update_fn_call_sig").data();
        const SigByte nop_call[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(check_for_update_fn_call_sig, nop_call);
        write_jmp_call(check_for_update_fn_call_sig, hook, reinterpret_cast<const void *>(block_update_check_asm), nullptr, false);

        auto *update_already_checked_sig = chimera.get_signature("update_already_checked_sig").data();
        update_check_already_checked = *reinterpret_cast<std::uint8_t **>(update_already_checked_sig + 14);
        const SigByte write_one_to_eax_here[] = { 0xB8, 0x01, 0x00, 0x00, 0x00 };
        write_code_s(update_already_checked_sig, write_one_to_eax_here);
    }
}
