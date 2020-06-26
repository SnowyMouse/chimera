// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_button_quotes_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(*argv);
            if(new_enabled != enabled) {
                auto &chimera = get_chimera();
                auto &left = chimera.get_signature("quote_left_sig");
                auto &right = chimera.get_signature("quote_right_sig");
                if(new_enabled) {
                    static auto *null_str = L"";
                    overwrite(left.data() + 1, null_str);
                    overwrite(right.data() + 1, null_str);
                }
                else {
                    left.rollback();
                    right.rollback();
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
