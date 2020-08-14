// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../config/ini.hpp"
#include "../../../halo_data/game_engine.hpp"
#include "../../../halo_data/hud_fonts.hpp"

namespace Chimera {
    #define CENTER_POS 300
    #define NAME_POS CENTER_POS - 70
    #define PLACE_POS NAME_POS - 50
    #define SCORE_POS CENTER_POS + 60
    #define PING_POS SCORE_POS + 60
    #define HEADER_POS PLACE_POS - 20

    bool simple_score_screen_command(int argc, const char **argv) {
        static bool simple_score_screen_active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != simple_score_screen_active) {
                auto &ss_elements_sig_a = get_chimera().get_signature("ss_elements_sig_a");
                auto &ss_elements_sig_b = get_chimera().get_signature("ss_elements_sig_b");
                auto &ss_score_background_sig = get_chimera().get_signature("ss_score_background_sig");
                auto &ss_score_position_sig = get_chimera().get_signature("ss_score_position_sig");

                auto *ss_elements_addr_b = ss_elements_sig_b.data();

                // Get the address from the jmp instruction of the scoreboard font hook
                auto jmp_offset = *reinterpret_cast<std::uint32_t *>(ss_elements_addr_b + 1);
                auto *jmp_address = reinterpret_cast<std::byte *>(ss_elements_addr_b + 5 + jmp_offset);
                auto *placement_position = jmp_address + 0x0A;

                simple_score_screen_active = new_value;
                if(new_value) {
                    auto *ss_elements_addr_a = ss_elements_sig_a.data();
                    overwrite(ss_elements_addr_a + 0, static_cast<std::uint8_t>(0xB9));
                    overwrite(ss_elements_addr_a + 1, (static_cast<std::uint16_t>(HEADER_POS) << 16) | (static_cast<uint16_t>(HEADER_POS)));
                    overwrite(ss_elements_addr_a + 5, static_cast<std::uint8_t>(0x90));

                    overwrite(placement_position, static_cast<std::int16_t>(PLACE_POS));           // placement
                    overwrite(ss_elements_addr_b + 7*1 + 5, static_cast<std::int16_t>(NAME_POS));  // name
                    overwrite(ss_elements_addr_b + 7*2 + 5, static_cast<std::int16_t>(SCORE_POS)); // score
                    overwrite(ss_elements_addr_b + 7*3 + 5, static_cast<std::int16_t>(0x6FFF));    // kills
                    overwrite(ss_elements_addr_b + 7*4 + 5, static_cast<std::int16_t>(0x6FFF));    // assists
                    overwrite(ss_elements_addr_b + 7*5 + 5, static_cast<std::int16_t>(0x6FFF));    // deaths
                    overwrite(ss_elements_addr_b + 7*6 + 2, static_cast<std::int16_t>(PING_POS));  // ping

                    auto *ss_score_position_addr = ss_score_position_sig.data();
                    overwrite(ss_score_position_addr + 2, static_cast<std::int8_t>(0x10));
                    overwrite(ss_score_position_addr + 7 + 2, static_cast<std::int8_t>(0x60));

                    SigByte nope[] = {0x90, 0x90, 0x90, 0x90, 0x90};
                    write_code_s(ss_score_background_sig.data() + 52, nope);
                }
                else {
                    ss_elements_sig_a.rollback();
                    ss_elements_sig_b.rollback();
                    ss_score_background_sig.rollback();
                    ss_score_position_sig.rollback();

                    set_up_scoreboard_font();
                }
            }
        }
        console_output(BOOL_TO_STR(simple_score_screen_active));
        return true;
    }
}
