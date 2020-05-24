#include "custom_map_lobby_fix.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../output/output.hpp"

namespace Chimera {
    void set_up_fp_reverb_fix(int fix) noexcept {
        auto &chimera = get_chimera();
        if(!chimera.feature_present("client_fp_reverb")) {
            return;
        }

        bool override_sound_class = fix == 1 || fix == 2;
        bool meme_up_positioning = fix == 2;

        auto *first_person_reverb_1 = chimera.get_signature("first_person_reverb_1_sig").data();
        auto *first_person_reverb_2 = chimera.get_signature("first_person_reverb_2_sig").data();
        auto *first_person_reverb_3 = chimera.get_signature("first_person_reverb_3_sig").data();

        if(meme_up_positioning) {
            static constexpr const SigByte nop_add[] = { 0x90, 0x90, 0x90 };
            write_code_s(first_person_reverb_1, nop_add);
            write_code_s(first_person_reverb_2, nop_add);
            write_code_s(first_person_reverb_3, nop_add);
        }

        if(override_sound_class) {
            static constexpr const SigByte tell_lies_on_the_internet[] = { 0x66, 0xB8, 0x0D, 0x00 };
            auto *first_person_reverb_4 = chimera.get_signature("first_person_reverb_4_sig").data();
            write_code_s(first_person_reverb_4, tell_lies_on_the_internet);
        }
    }

    void disable_fp_reverb_fix() noexcept {
        auto &chimera = get_chimera();
        chimera.get_signature("first_person_reverb_1_sig").rollback();
        chimera.get_signature("first_person_reverb_2_sig").rollback();
        chimera.get_signature("first_person_reverb_3_sig").rollback();
        chimera.get_signature("first_person_reverb_4_sig").rollback();
    }

    bool test_fp_reverb_fix_command(int argc, const char **argv) {
        static int current_setting = 0;
        if(argc) {
            disable_fp_reverb_fix();
            int new_setting = std::strtol(argv[0], nullptr, 10);
            if(new_setting < 0 || new_setting > 2) {
                new_setting = 0;
            }
            set_up_fp_reverb_fix(new_setting);
            current_setting = new_setting;
        }
        switch(current_setting) {
            case 0:
                console_output("0 - off");
                break;
            case 1:
                console_output("1 - on (global)");
                break;
            case 2:
                console_output("2 - on (global - disable node offset)");
                break;
        }
        return true;
    }
}
