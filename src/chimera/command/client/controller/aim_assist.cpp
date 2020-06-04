// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool aim_assist_command(int argc, const char **argv) {
        static auto &active = **reinterpret_cast<char **>(get_chimera().get_signature("aim_assist_enabled_sig").data() + 1);
        if(argc == 1) {
            active = STR_TO_BOOL(argv[0]);
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
