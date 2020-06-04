// SPDX-License-Identifier: GPL-3.0-only

#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../fix/auto_center.hpp"
#include "../../../localization/localization.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_auto_center_command(int argc, const char **argv) {
        static bool setting = false;
        if(argc == 1) {
            bool val = STR_TO_BOOL(argv[0]);
            if(val != setting) {
                set_up_auto_center_fix(val);
                setting = val;
            }
        }
        console_output("%s", setting ? "true" : "false");
        return true;
    }
}
