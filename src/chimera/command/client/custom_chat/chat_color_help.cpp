// SPDX-License-Identifier: GPL-3.0-only

#include "../../../custom_chat/custom_chat.hpp"
#include "../../../command/command.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool chat_color_help_command(int argc, const char **argv) {
        if(argc) {
            set_show_color_help(STR_TO_BOOL(*argv));
        }
        console_output(BOOL_TO_STR(show_color_help()));
        return true;
    }
}
