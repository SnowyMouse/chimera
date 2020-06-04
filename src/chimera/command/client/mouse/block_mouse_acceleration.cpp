// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_mouse_acceleration_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &mouse_accel_1_sig = get_chimera().get_signature("mouse_accel_1_sig");
                auto &mouse_accel_2_sig = get_chimera().get_signature("mouse_accel_2_sig");
                if(new_enabled) {
                    static float zero = 0;

                    // Overwrite code, removing mouse acceleration.
                    overwrite(mouse_accel_1_sig.data(), static_cast<unsigned char>(0xEB));
                    overwrite(mouse_accel_2_sig.data() + 1, static_cast<unsigned char>(0x05));
                    overwrite(mouse_accel_2_sig.data() + 2, &zero);
                    overwrite(mouse_accel_2_sig.data() + 6 + 1, static_cast<unsigned char>(0x25));
                    overwrite(mouse_accel_2_sig.data() + 6 + 2, &zero);
                }
                else {
                    mouse_accel_1_sig.rollback();
                    mouse_accel_2_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
