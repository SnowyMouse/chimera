// SPDX-License-Identifier: GPL-3.0-only

#include "../../../custom_chat/custom_chat.hpp"
#include "../../../command/command.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool chat_block_server_messages_command(int argc, const char **argv) {
        if(argc) {
            set_server_messages_blocked(STR_TO_BOOL(*argv));
        }
        console_output(BOOL_TO_STR(server_messages_blocked()));
        return true;
    }
}
