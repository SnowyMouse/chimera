// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool camo_fix_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                static std::uint32_t amazing_camo_fix_value = 0;
                auto &alpha_blend_camo_sig = get_chimera().get_signature("alpha_blend_camo_sig");
                auto &dart_1_sig = get_chimera().get_signature("dart_1_sig");
                auto &dart_2_sig = get_chimera().get_signature("dart_2_sig");
                auto &nvidia_camo_1_sig = get_chimera().get_signature("nvidia_camo_1_sig");
                auto &nvidia_camo_2_sig = get_chimera().get_signature("nvidia_camo_2_sig");
                auto &nvidia_camo_3_sig = get_chimera().get_signature("nvidia_camo_3_sig");

                if(new_enabled) {
                    overwrite(alpha_blend_camo_sig.data() + 4, static_cast<float>(1.0 - 0.9 / std::pow(1.34, 13.4)));
                    overwrite(dart_1_sig.data() + 2, &amazing_camo_fix_value);
                    overwrite(dart_1_sig.data() + 9, &amazing_camo_fix_value);
                    overwrite(dart_1_sig.data() + 16, &amazing_camo_fix_value);
                    overwrite(dart_1_sig.data() + 24, &amazing_camo_fix_value);

                    const SigByte skip_dart[] = { 0xEB, 0x10, 0x90, 0x90, 0x90, 0x90 };
                    write_code_s(dart_2_sig.data(), skip_dart);

                    const SigByte fuck_camo_fix_0_in_hac2_why_the_fuck_would_you_let_people_turn_it_off_even_on_newer_versions_of_the_game_where_it_doesnt_fucking_matter[] = { 0x30, 0xC9, 0x90, 0x90, 0x90, 0x90 };
                    write_code_s(nvidia_camo_2_sig.data(), fuck_camo_fix_0_in_hac2_why_the_fuck_would_you_let_people_turn_it_off_even_on_newer_versions_of_the_game_where_it_doesnt_fucking_matter);

                    const SigByte also_fuck_people_who_use_it_because_they_are_complete_shit_at_the_game_go_back_to_minecraft_you_scrubs[] = { 0x30, 0xC0, 0x90, 0x90, 0x90 };
                    write_code_s(nvidia_camo_1_sig.data() + 3, also_fuck_people_who_use_it_because_they_are_complete_shit_at_the_game_go_back_to_minecraft_you_scrubs);

                    const SigByte and_fuck_people_who_think_that_chimera_should_not_have_this_because_they_have_no_fucking_clue_what_they_are_talking_about[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
                    write_code_s(nvidia_camo_3_sig.data(), and_fuck_people_who_think_that_chimera_should_not_have_this_because_they_have_no_fucking_clue_what_they_are_talking_about);
                }
                else {
                    alpha_blend_camo_sig.rollback();
                    dart_1_sig.rollback();
                    dart_2_sig.rollback();
                    nvidia_camo_1_sig.rollback();
                    nvidia_camo_2_sig.rollback();
                    nvidia_camo_3_sig.rollback();
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
