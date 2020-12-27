// SPDX-License-Identifier: GPL-3.0-only

#include "extend_limits.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../output/output.hpp"

#include <cstdint>
#include <memory>

#define VISIBLE_OBJECT_LIMIT 1024

extern "C" {
    std::uint32_t *extend_limits_polies_count = nullptr;
    std::uint32_t extend_limits_poly_limit = BSP_POLY_LIMIT;
    const void *extend_limits_count_1 = nullptr;
    const void *extend_limits_count_2 = nullptr;
    const void *extend_limits_count_3 = nullptr;

    void extend_limits_cmp_1() noexcept;
    void extend_limits_cmp_2() noexcept;
    void extend_limits_cmp_3() noexcept;
}

namespace Chimera {
    static auto bsp_polies = std::make_unique<std::uint32_t []>(BSP_POLY_LIMIT);
    void set_up_extend_limits() noexcept {
        // There's a ton of stuff here.
        static const float MAXIMUM_DRAW_DISTANCE = 1026.0F * 1.34F * 1.34F * 1.34F * 1.34F * 1.34F / 1.97F;
        static auto visible_objects = std::make_unique<std::uint32_t []>(VISIBLE_OBJECT_LIMIT);

        auto &bsp_poly_1_sig = get_chimera().get_signature("bsp_poly_1_sig");
        auto &bsp_poly_2_sig = get_chimera().get_signature("bsp_poly_2_sig");
        auto &bsp_poly_3_sig = get_chimera().get_signature("bsp_poly_3_sig");
        auto &bsp_poly_4_sig = get_chimera().get_signature("bsp_poly_4_sig");
        auto &bsp_poly_5_sig = get_chimera().get_signature("bsp_poly_5_sig");
        auto &bsp_poly_6_sig = get_chimera().get_signature("bsp_poly_6_sig");
        auto &bsp_poly_7_sig = get_chimera().get_signature("bsp_poly_7_sig");
        auto &bsp_poly_8_sig = get_chimera().get_signature("bsp_poly_8_sig");
        auto &bsp_poly_9_sig = get_chimera().get_signature("bsp_poly_9_sig");
        auto &bsp_poly_10_sig = get_chimera().get_signature("bsp_poly_10_sig");
        auto &bsp_poly_11_sig = get_chimera().get_signature("bsp_poly_11_sig");
        auto &bsp_poly_12_sig = get_chimera().get_signature("bsp_poly_12_sig");
        auto &bsp_poly_13_sig = get_chimera().get_signature("bsp_poly_13_sig");

        auto &bsp_poly_count_sig = get_chimera().get_signature("bsp_poly_count_sig");
        auto &bsp_poly_limit_1_sig = get_chimera().get_signature("bsp_poly_limit_1_sig");
        auto &bsp_poly_limit_2_sig = get_chimera().get_signature("bsp_poly_limit_2_sig");
        auto &bsp_poly_limit_3_sig = get_chimera().get_signature("bsp_poly_limit_3_sig");
        auto &bsp_poly_movsx_sig = get_chimera().get_signature("bsp_poly_movsx_sig");
        auto &bsp_poly_movsx_2_sig = get_chimera().get_signature("bsp_poly_movsx_2_sig");

        auto &visible_object_list_1_sig = get_chimera().get_signature("visible_object_list_1_sig");
        auto &visible_object_list_2_sig = get_chimera().get_signature("visible_object_list_2_sig");
        auto &visible_object_list_3_sig = get_chimera().get_signature("visible_object_list_3_sig");
        auto &visible_object_ptr_sig = get_chimera().get_signature("visible_object_ptr_sig");

        auto &visible_object_limit_1_sig = get_chimera().get_signature("visible_object_limit_1_sig");
        auto &visible_object_limit_2_sig = get_chimera().get_signature("visible_object_limit_2_sig");

        auto &draw_distance_sig = get_chimera().get_signature("draw_distance_sig");

        // Chimera allocated a new BSP polygon array. We need to point to those.
        overwrite(bsp_poly_1_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_2_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_3_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_4_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_5_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_6_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_7_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_8_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_9_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_10_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_11_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_12_sig.data() + 1, bsp_polies.get());
        overwrite(bsp_poly_13_sig.data() + 1, bsp_polies.get());

        // What I'm doing is making the polygon count a 32-bit integer
        extend_limits_polies_count = *reinterpret_cast<std::uint32_t **>(bsp_poly_count_sig.data() + 3);
        overwrite(bsp_poly_count_sig.data(), static_cast<std::uint8_t>(0xFF));
        overwrite(bsp_poly_count_sig.data() + 1, static_cast<std::uint8_t>(0x05));
        overwrite(bsp_poly_count_sig.data() + 2, extend_limits_polies_count);
        overwrite(bsp_poly_count_sig.data() + 6, static_cast<std::uint8_t>(0x90));

        if(get_chimera().feature_present("client_bsp_poly_demo")) { //005076AD
            auto &bsp_poly_count_demo_sig = get_chimera().get_signature("bsp_poly_demo_sig");
            overwrite(bsp_poly_count_demo_sig.data() + 1, bsp_polies.get());
        }

        SigByte nop_bsp_limit[9] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(bsp_poly_limit_1_sig.data(), nop_bsp_limit);
        write_code_s(bsp_poly_limit_2_sig.data(), nop_bsp_limit);
        write_code_s(bsp_poly_limit_3_sig.data(), nop_bsp_limit);
        static Hook bsp_limit_1_hook;
        static Hook bsp_limit_2_hook;
        static Hook bsp_limit_3_hook;
        write_function_override(bsp_poly_limit_1_sig.data(), bsp_limit_1_hook, reinterpret_cast<const void *>(extend_limits_cmp_1), &extend_limits_count_1);
        write_function_override(bsp_poly_limit_2_sig.data(), bsp_limit_2_hook, reinterpret_cast<const void *>(extend_limits_cmp_2), &extend_limits_count_2);
        write_function_override(bsp_poly_limit_3_sig.data(), bsp_limit_3_hook, reinterpret_cast<const void *>(extend_limits_cmp_3), &extend_limits_count_3);

        SigByte nop_movsx[3] = {0x90, 0x90, 0x90};
        write_code_s(bsp_poly_movsx_sig.data(), nop_movsx);

        SigByte new_movsx[] =  { -1, -1, -1, 0x85, 0xFF, 0x90, -1, -1, -1, -1, -1, -1, -1, 0x8B, 0xD7, 0x90 };
        write_code_s(bsp_poly_movsx_2_sig.data(), new_movsx);

        // Bump up the visible object limit
        overwrite(visible_object_list_1_sig.data() + 3, visible_objects.get());
        overwrite(visible_object_list_2_sig.data() + 1, static_cast<std::uint32_t>(VISIBLE_OBJECT_LIMIT));
        overwrite(visible_object_list_2_sig.data() + 5 + 1, visible_objects.get());
        overwrite(visible_object_list_3_sig.data() + 3, visible_objects.get());
        overwrite(visible_object_ptr_sig.data() + 3, visible_objects.get());
        overwrite(visible_object_limit_1_sig.data() + 7, static_cast<std::uint16_t>(VISIBLE_OBJECT_LIMIT));
        overwrite(visible_object_limit_2_sig.data() + 1, static_cast<std::uint32_t>(VISIBLE_OBJECT_LIMIT));

        overwrite(draw_distance_sig.data() + 1, &MAXIMUM_DRAW_DISTANCE);
    }
}
