// SPDX-License-Identifier: GPL-3.0-only

#include "../../../custom_chat/custom_chat.hpp"
#include "../../../command/command.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool chat_block_ips_command(int argc, const char **argv) {
        if(argc) {
            set_block_ips(STR_TO_BOOL(*argv));
        }
        console_output(BOOL_TO_STR(ips_blocked()));
        return true;
    }
}
