// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool meme_zone_command(int argc, const char **argv) {
        float value = 1.0F;
        if(argc == 1) {
            try {
                value = std::stof(argv[0]);
            }
            catch(std::exception &) {
                value = 0.0F;
            }
            if(value <= 1.0F) {
                value = 1.0F;
            }

            overwrite(get_chimera().get_signature("widescreen_text_scaling_sig").data() + 0x9C, value);
            overwrite(get_chimera().get_signature("widescreen_element_motion_sensor_scaling_sig").data() + 0xA6, value);
            overwrite(get_chimera().get_signature("widescreen_element_scaling_sig").data() + 0x96, value);
        }
        console_output("%f", value);
        return true;
    }
}
