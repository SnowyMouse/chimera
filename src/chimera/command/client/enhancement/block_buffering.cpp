// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_buffering_command(int argc, const char **argv) {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                auto &setting = **reinterpret_cast<char **>(get_chimera().get_signature("disable_buffering_sig").data() + 1);
                if(new_value && setting) {
                    console_warning(localize("chimera_block_buffering_command_warning"));
                }
                overwrite(&setting, static_cast<char>(new_value));
                active = new_value;
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
