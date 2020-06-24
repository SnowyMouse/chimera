// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void remove_watson() noexcept {
        auto *data = get_chimera().get_signature("create_watson_process_sig").data();

        const SigByte jmp[] = { 0xEB, 0x25, 0x90 };
        write_code_s(data, jmp);

        const SigByte xor_eax_eax[] = { 0x90, 0x90, 0x90, 0x90, 0x31, 0xC0 };
        write_code_s(data + 35, xor_eax_eax);
    }
}
