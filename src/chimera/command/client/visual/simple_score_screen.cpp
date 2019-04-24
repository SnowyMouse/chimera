#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool simple_score_screen_command(int argc, const char **argv) {
        static bool simple_score_screen_active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != simple_score_screen_active) {
                auto &ss_elements_sig = get_chimera().get_signature("ss_elements_sig");
                auto &ss_score_background_sig = get_chimera().get_signature("ss_score_background_sig");
                auto &ss_score_position_sig = get_chimera().get_signature("ss_score_position_sig");
                simple_score_screen_active = new_value;
                if(new_value) {

                    #define CENTER_POS 300
                    #define NAME_POS CENTER_POS - 70
                    #define PLACE_POS NAME_POS - 50
                    #define SCORE_POS CENTER_POS + 60
                    #define PING_POS SCORE_POS + 60
                    #define HEADER_POS PLACE_POS - 20

                    auto *ss_elements_addr = ss_elements_sig.data();
                    overwrite(ss_elements_addr + 0, static_cast<unsigned char>(0xB9));
                    overwrite(ss_elements_addr + 1, (static_cast<uint16_t>(HEADER_POS) << 16) | (static_cast<uint16_t>(HEADER_POS)));
                    overwrite(ss_elements_addr + 5, static_cast<unsigned char>(0x90));
                    overwrite(ss_elements_addr + 0x2E + 7*0 + 5, static_cast<short>(PLACE_POS)); // placement
                    overwrite(ss_elements_addr + 0x2E + 7*1 + 5, static_cast<short>(NAME_POS)); // name
                    overwrite(ss_elements_addr + 0x2E + 7*2 + 5, static_cast<short>(SCORE_POS)); // score
                    overwrite(ss_elements_addr + 0x2E + 7*3 + 5, static_cast<short>(0x6FFF)); // kills
                    overwrite(ss_elements_addr + 0x2E + 7*4 + 5, static_cast<short>(0x6FFF)); // assists
                    overwrite(ss_elements_addr + 0x2E + 7*5 + 5, static_cast<short>(0x6FFF)); // deaths
                    overwrite(ss_elements_addr + 0x2E + 7*6 + 2, static_cast<short>(PING_POS)); // ping

                    auto *ss_score_position_addr = ss_score_position_sig.data();
                    overwrite(ss_score_position_addr + 2, static_cast<char>(0x10));
                    overwrite(ss_score_position_addr + 7 + 2, static_cast<char>(0x60));

                    SigByte nope[] = {0x90, 0x90, 0x90, 0x90, 0x90};
                    write_code_s(ss_score_background_sig.data() + 52, nope);
                }
                else {
                    ss_elements_sig.rollback();
                    ss_score_background_sig.rollback();
                    ss_score_position_sig.rollback();
                }
            }
        }
        console_output(BOOL_TO_STR(simple_score_screen_active));
        return true;
    }
}
