// SPDX-License-Identifier: GPL-3.0-only

#include "sun_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/output.hpp"
#include "../halo_data/resolution.hpp"
#include "../event/frame.hpp"

namespace Chimera {
    static float z = 0.0625;

    static void correct_sun() noexcept;

    void set_up_sun_fix() noexcept {
        if(!get_chimera().feature_present("client_sun")) {
            return;
        }
        auto *sun = get_chimera().get_signature("lens_scale_sig").data();
        overwrite(sun + 2, &z);
        add_preframe_event(correct_sun);
    }

    static void correct_sun() noexcept {
        // Since Halo PC scales sun by 768p (even though its default res is 800x600 and supports custom resolution), we scale lens flares by dividing by 768
        static std::uint16_t height = 0;
        auto new_height = get_resolution().height;
        if(new_height != height) {
            height = new_height;
            float scale = static_cast<float>(new_height) / 768.0F;
            z = scale * 0.0625;
        }
    }
}
