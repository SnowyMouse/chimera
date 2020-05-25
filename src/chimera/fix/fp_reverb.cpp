#include "custom_map_lobby_fix.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../output/output.hpp"
#include "../halo_data/camera.hpp"
#include "../halo_data/player.hpp"
#include "../halo_data/object.hpp"

namespace Chimera {
    extern "C" void override_non_firing_fp_reverb_asm() noexcept;
    extern "C" void meme_up_reverb_position_asm() noexcept;

    extern "C" void have_fun_with_positioning(std::uint32_t oid_whole_id, float *v) noexcept {
        // If we're not in first person, don't do anything
        if(camera_type() != CameraType::CAMERA_FIRST_PERSON) {
            return;
        }

        // Also, are we alive? If so, get our object.
        auto *player = PlayerTable::get_player_table().get_client_player();
        if(!player) {
            return;
        }
        auto &ot = ObjectTable::get_object_table();
        auto *player_object = reinterpret_cast<UnitDynamicObject *>(ot.get_dynamic_object(player->object_id));
        if(!player_object) {
            return;
        }

        // Also, if this object is NOT our weapon or us, don't do anything
        if((oid_whole_id != player->object_id.whole_id && oid_whole_id != player_object->weapon.whole_id) || oid_whole_id == 0xFFFFFFFF) {
            return;
        }

        auto &d = camera_data();
        v[0] = d.position.x;
        v[1] = d.position.y;
        v[2] = d.position.z;
    }

    void set_up_fp_reverb_fix(int fix) noexcept {
        auto &chimera = get_chimera();
        if(!chimera.feature_present("client_fp_reverb")) {
            return;
        }

        bool override_sound_class = fix == 1 || fix == 2;
        bool meme_up_positioning = fix == 2;
        bool ignore_firing = fix == 3;

        auto *first_person_reverb_4 = chimera.get_signature("first_person_reverb_4_sig").data();
        auto *first_person_reverb_5 = chimera.get_signature("first_person_reverb_5_sig").data();

        if(meme_up_positioning) {
            //static constexpr const SigByte nop_add[] = { 0x90, 0x90, 0x90 };
            //write_code_s(first_person_reverb_1, nop_add);
            //write_code_s(first_person_reverb_2, nop_add);
            //write_code_s(first_person_reverb_3, nop_add);

            static Hook hook;
            write_jmp_call(reinterpret_cast<std::byte *>(first_person_reverb_5), hook, nullptr, reinterpret_cast<const void *>(meme_up_reverb_position_asm), false);
            //write_jmp_call(first_person_reverb_2, b, reinterpret_cast<const void *>(null_equipment_spawn_rotation1), nullptr, false);
            //write_jmp_call(first_person_reverb_3, c, reinterpret_cast<const void *>(null_equipment_spawn_rotation1), nullptr, false);
        }

        if(override_sound_class) {
            static constexpr const SigByte tell_lies_on_the_internet[] = { 0x66, 0xB8, 0x0D, 0x00 };
            write_code_s(first_person_reverb_4, tell_lies_on_the_internet);
        }

        if(ignore_firing) {
            static Hook hook;
            write_jmp_call(first_person_reverb_4, hook, nullptr, reinterpret_cast<const void *>(override_non_firing_fp_reverb_asm), false);
        }
    }

    void disable_fp_reverb_fix() noexcept {
        auto &chimera = get_chimera();
        chimera.get_signature("first_person_reverb_1_sig").rollback();
        chimera.get_signature("first_person_reverb_2_sig").rollback();
        chimera.get_signature("first_person_reverb_3_sig").rollback();
        chimera.get_signature("first_person_reverb_4_sig").rollback();
        chimera.get_signature("first_person_reverb_5_sig").rollback();
    }

    bool test_fp_reverb_fix_command(int argc, const char **argv) {
        static int current_setting = 0;
        if(argc) {
            disable_fp_reverb_fix();
            int new_setting = std::strtol(argv[0], nullptr, 10);
            if(new_setting < 0 || new_setting > 3) {
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
                console_output("2 - on (global - set offset to FP camera)");
                break;
            case 3:
                console_output("3 - on (global - exclude firing sounds)");
                break;
        }
        return true;
    }
}
