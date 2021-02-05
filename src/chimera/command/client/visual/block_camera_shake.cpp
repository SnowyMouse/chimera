// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_camera_shake_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &block_camera_shake_sig = get_chimera().get_signature("block_camera_shake_sig");
                if(new_enabled) {
                    overwrite(block_camera_shake_sig.data(), static_cast<std::uint8_t>(0x90));
                    overwrite(block_camera_shake_sig.data() + 1, static_cast<std::uint8_t>(0xE9));
                }
                else {
                    block_camera_shake_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
