// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool devmode_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &devmode_sig = get_chimera().get_signature("devmode_sig");
                if(new_enabled) {
                    const SigByte force_devmode[] = { 0x90, 0x90, -1, -1, 0x90, 0x90, -1, -1, -1, -1, -1 };
                    write_code_s(devmode_sig.data(), force_devmode);
                }
                else {
                    devmode_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }

    bool devmode_retail_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &devmode_sig = get_chimera().get_signature("devmode_retail_sig");
                if(new_enabled) {
                    const SigByte force_devmode[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0xB0, 0x01, -1 };
                    write_code_s(devmode_sig.data(), force_devmode);
                }
                else {
                    devmode_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
