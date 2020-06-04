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
}

namespace Chimera {
    static void set_values();
    void set_up_model_detail_fix() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("model_lod_sig").data(), hook, reinterpret_cast<const void *>(model_detail_fix));
        add_preframe_event(set_values);
    }

    static float model_detail = 1.0F;

    static void set_values() {
        static std::uint16_t height = 0;
        auto resolution = get_resolution();
        if(resolution.height != height) {
            set_model_detail(model_detail);
        }
    }

    void set_model_detail(float detail) noexcept {
        if(detail < 0.01F) {
            detail = 0.01F;
        }
        model_detail = detail;
        lod_fix_scale = static_cast<float>(get_resolution().height) / 640.0F / model_detail;
    }

    float get_model_detail() noexcept {
        return model_detail;
    }
}
