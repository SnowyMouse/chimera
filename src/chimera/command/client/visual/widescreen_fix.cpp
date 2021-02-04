// SPDX-License-Identifier: GPL-3.0-only

#include "../../../fix/widescreen_fix.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../halo_data/hud_fonts.hpp"
#include "../../command.hpp"

namespace Chimera {
    bool widescreen_fix_command(int argc, const char **argv) {
        static auto setting = WidescreenFixSetting::WIDESCREEN_OFF;
        if(argc) {
            int new_setting = std::atoi(argv[0]);
            if(new_setting < 0) {
                new_setting = 0;
            }
            else if(new_setting > 2) {
                new_setting = 2;
            }
            if(new_setting == 0 && BOOL_TO_STR(argv[0])) {
                new_setting = 1;
            }
            
            setting = static_cast<WidescreenFixSetting>(new_setting);
            
            if(!setting && hud_text_mod_initialized()) {
                console_warning(localize("chimera_widescreen_fix_command_warning_cannot_disable_font_override_enabled"));
            }
            else {
                set_widescreen_fix(setting);
            }
        }
        console_output("%i", setting);
        return true;
    }
}
