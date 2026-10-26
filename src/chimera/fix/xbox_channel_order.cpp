// SPDX-License-Identifier: GPL-3.0-only

#include "xbox_channel_order.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/shader_defs.hpp"

namespace Chimera {
    extern "C" {
        void set_psh_const_for_xbox_order_asm() noexcept;
        void set_psh_const_for_cc_error_asm() noexcept;
    }

    static float *psh_constants = nullptr;

    extern "C" void set_psh_const_for_xbox_order(ShaderModel *shader) noexcept {
        if(TEST_FLAG(shader->model.flags, SHADER_MODEL_FLAGS_USE_XBOX_MULTIPURPOSE_CHANNEL_ORDER_BIT)) {
            // gearbox defaults this to 1.0f, which is opposite of MCC. The shader code should be set up accordingly.
            psh_constants[19] = 0.0f;
        }
    }

    extern "C" void set_psh_const_for_cc_error_red(float cc0_r) noexcept {
        // This doesn't get set for whatever reason, so when the camera is in the fog plane, obscured models can have the wrong color.
        psh_constants[8] = PIN(-cc0_r, 0.0f, 1.0f);
    }

    void set_up_xbox_channel_order_support() noexcept {
        auto set_shader_const_ptr = get_chimera().get_signature("use_xbox_channel_order_sig").data();
        
        // The channel order is determined by the alpha channel of this shader constant.
        psh_constants = reinterpret_cast<float *>(*reinterpret_cast<std::byte **>(set_shader_const_ptr + 2)) - 19;

        static Hook hook;
        write_jmp_call(set_shader_const_ptr + 10, hook, reinterpret_cast<const void*>(set_psh_const_for_xbox_order_asm), nullptr);

        // Fucking gearbox.... Just tack this on in here to fix a meme.
        if(get_chimera().feature_present("client_custom_edition")) {
            static Hook hook2;
            auto *set_cc0 = get_chimera().get_signature("model_draw_set_cc0_error_sig").data() + 4;
            write_jmp_call(set_cc0, hook2, reinterpret_cast<const void *>(set_psh_const_for_cc_error_asm), nullptr);
        }
    }
}
