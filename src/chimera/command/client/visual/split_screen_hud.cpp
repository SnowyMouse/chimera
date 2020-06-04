// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool split_screen_hud_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &ammo_counter_ss_sig = get_chimera().get_signature("ammo_counter_ss_sig");
                auto &hud_text_ss_sig = get_chimera().get_signature("hud_text_ss_sig");
                auto &split_screen_hud_ss_sig = get_chimera().get_signature("split_screen_hud_ss_sig");
                if(new_enabled) {
                    const short ammo_counter_mod[] = {-1,   0xB8, 0x02, 0x00};
                    const short hud_text_mod[] = {-1,   0xB8, 0x02, 0x00};
                    const short split_screen_hud_mod[] = {-1,   -1,   -1,   -1,   0x00};
                    write_code_s(ammo_counter_ss_sig.data(), ammo_counter_mod);
                    write_code_s(hud_text_ss_sig.data(), hud_text_mod);
                    write_code_s(split_screen_hud_ss_sig.data(), split_screen_hud_mod);
                }
                else {
                    ammo_counter_ss_sig.rollback();
                    hud_text_ss_sig.rollback();
                    split_screen_hud_ss_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
