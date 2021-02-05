// SPDX-License-Identifier: GPL-3.0-only

#include "video_mode.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

#include <cstdint>


namespace Chimera {
    extern "C" void invert_fog_atmospheric_dominant_flag_asm();

    void set_up_inverted_flag_fix() noexcept {
        auto &shader_model_detail_after_reflection_sig = get_chimera().get_signature("shader_model_detail_after_reflection_sig");
        overwrite(shader_model_detail_after_reflection_sig.data(), static_cast<std::uint8_t>(0x75)); // flip je <-> jn

        static Hook atmosphere_fog_flags_hook;
        auto &atmosphere_fog_flags_sig = get_chimera().get_signature("atmosphere_fog_flags_sig");
        write_jmp_call(atmosphere_fog_flags_sig.data() + 3, atmosphere_fog_flags_hook, reinterpret_cast<const void *>(invert_fog_atmospheric_dominant_flag_asm), nullptr, false);
    }

    void unset_up_inverted_flag_fix() noexcept {
        get_chimera().get_signature("shader_model_detail_after_reflection_sig").rollback();
        get_chimera().get_signature("atmosphere_fog_flags_sig").rollback();
    }
}
