// SPDX-License-Identifier: GPL-3.0-only

#include "../../../fix/widescreen_fix.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../custom_chat/hud_text.hpp"
#include "../../command.hpp"

namespace Chimera {
    bool widescreen_fix_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            enabled = STR_TO_BOOL(*argv);
            if(!enabled && hud_text_mod_initialized()) {
                console_warning(localize("chimera_widescreen_fix_command_warning_cannot_disable_font_override_enabled"));
            }
            else {
                set_widescreen_fix(enabled);
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
