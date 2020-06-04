// SPDX-License-Identifier: GPL-3.0-only

#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include "../../../event/tick.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"

namespace Chimera {
    static float **letterbox;
    static void block_letterbox() noexcept {
        **letterbox = -0.5;
    }

    bool block_letterbox_command(int argc, const char **argv) noexcept {
        static auto active = false;
        if(argc) {
            bool new_value = STR_TO_BOOL(argv[0]);
            letterbox = *reinterpret_cast<float ***>(get_chimera().get_signature("letterbox_sig").data() + 2);
            if(new_value != active) {
                active = new_value;
                if(active) {
                    add_tick_event(block_letterbox);
                }
                else {
                    remove_tick_event(block_letterbox);
                }
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
