// SPDX-License-Identifier: GPL-3.0-only

#include "xbox_channel_order.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/shader_defs.hpp"

namespace Chimera {
    extern "C" void set_psh_const_for_xbox_order_asm() noexcept;
    static float *c_self_illumination_color_w = nullptr;

    extern "C" void set_psh_const_for_xbox_order(ShaderModel *shader) noexcept {
        if(TEST_FLAG(shader->model.flags, SHADER_MODEL_FLAGS_USE_XBOX_MULTIPURPOSE_CHANNEL_ORDER_BIT)) {
            // gearbox defaults this to 1.0f, which is opposite of MCC. The shader code should be set up accordingly.
            *c_self_illumination_color_w = 0.0f;
        }
    }

    void set_up_xbox_channel_order_support() noexcept {
        auto set_shader_const_ptr = get_chimera().get_signature("use_xbox_channel_order_sig").data();
        
        // The channel order is determined by the alpha channel of this shader constant.
        c_self_illumination_color_w = reinterpret_cast<float *>(*reinterpret_cast<std::byte **>(set_shader_const_ptr + 2));

        static Hook hook;
        write_jmp_call(set_shader_const_ptr + 10, hook, reinterpret_cast<const void*>(set_psh_const_for_xbox_order_asm), nullptr);
    }
}
