// SPDX-License-Identifier: GPL-3.0-only

#include "model_detail.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/resolution.hpp"
#include "../event/frame.hpp"

extern "C" {
    float lod_fix_scale = 1.0F;
    void model_detail_fix() noexcept;
    const void *model_detail_fix_original_instr = nullptr;
    std::byte *model_detail_set_lod_skip = nullptr;
    bool force_max_lod_level = false;
}

namespace Chimera {
    extern "C" void set_model_detail_scale() noexcept {
        lod_fix_scale = 1.0f;

        static std::uint16_t height = 0;
        auto resolution = get_resolution();
        if(resolution.height != height) {
            lod_fix_scale = static_cast<float>(resolution.height) / 480.0F;
        }
    }

    void set_up_model_detail_fix() noexcept {
        model_detail_set_lod_skip = get_chimera().get_signature("model_lod_skip_sig").data();
        auto *model_lod_ptr = get_chimera().get_signature("model_lod_sig").data() + 5;

        SigByte mod[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(model_lod_ptr, mod);

        static Hook hook;
        write_function_override(model_lod_ptr, hook, reinterpret_cast<const void *>(model_detail_fix), &model_detail_fix_original_instr);
    }
}
