// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../math_trig/math_trig.hpp"
#include "../../../fix/hud_bitmap_scale.hpp"

namespace Chimera {
    bool safe_zone_command(int argc, const char **argv) {
        static bool active = false;

        static float horiz = 0;
        static float vert = 0;
        if(argc == 2) {
            horiz = PIN(fast_ftol(strtof(argv[0], nullptr)), HUD_MARGIN, XBOX_SAFE_ZONE_WIDTH);
            vert = PIN(fast_ftol(strtof(argv[1], nullptr)), HUD_MARGIN, XBOX_SAFE_ZONE_HEIGHT);

            set_safe_zone_margins(horiz, vert);
            active = true;
        }
        else if(argc == 1) {
            bool enabled = STR_TO_BOOL(argv[0]);
            if(enabled) {
                horiz = XBOX_SAFE_ZONE_WIDTH;
                vert = XBOX_SAFE_ZONE_HEIGHT;
                set_safe_zone_margins(horiz, vert);
                active = true;
            }
            else {
                set_safe_zone_margins(HUD_MARGIN, HUD_MARGIN);
                active = false;
            }
        }
        
        if(active) {
            console_output("%.0f, %.0f", horiz, vert);
        }
        else {
            console_output("off");
        }
        return true;
    }
}
