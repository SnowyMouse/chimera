// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void remove_registry_checks() noexcept {
        SigByte load_eula_code[] = { 0xE9, 0x93, 0x00, 0x00, 0x00 };
        write_code_s(get_chimera().get_signature("load_eula_sig").data(), load_eula_code);
        overwrite(get_chimera().get_signature("registry_check_1_sig").data(), static_cast<std::uint8_t>(0xC3));

        SigByte registry_check_2_code[] = { 0x31, 0xC0, 0xC3 };
        write_code_s(get_chimera().get_signature("registry_check_2_sig").data(), registry_check_2_code);
        write_code_s(get_chimera().get_signature("registry_check_3_sig").data(), registry_check_2_code);
        write_code_s(get_chimera().get_signature("registry_check_4_sig").data(), registry_check_2_code);
        write_code_s(get_chimera().get_signature("gamma_sig").data(), registry_check_2_code);
    }
}
