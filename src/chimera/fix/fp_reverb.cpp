// SPDX-License-Identifier: GPL-3.0-only

#include "custom_map_lobby_fix.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../output/output.hpp"
#include "../halo_data/camera.hpp"
#include "../halo_data/player.hpp"
#include "../halo_data/object.hpp"

namespace Chimera {
    extern "C" void override_fp_reverb_position_asm() noexcept;
    extern "C" void move_current_fp_sound_to_camera(std::uint32_t object_id, Point3D *sound_position) noexcept {
        // If we're not in first person, don't do anything
        if(camera_type() != CameraType::CAMERA_FIRST_PERSON) {
            return;
        }

        // If it's not a thing, go away
        if(object_id == 0xFFFFFFFF) {
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

        // Also, if this object is NOT our body, check if it's our weapon
        if(object_id != player->object_id.whole_id) {
            bool is_us = false;
            for(auto &w : player_object->weapons) {
                if(w.whole_id == object_id) {
                    is_us = true;
                    break;
                }
            }
            if(!is_us) {
                return;
            }
        }

        auto &d = camera_data();
        *sound_position = d.position;
    }

    void set_up_fp_reverb_fix() noexcept {
        auto &chimera = get_chimera();
        if(!chimera.feature_present("client_fp_reverb")) {
            return;
        }

        auto *first_person_reverb_1 = chimera.get_signature("first_person_reverb_1_sig").data();
        auto *first_person_reverb_2 = chimera.get_signature("first_person_reverb_2_sig").data();

        // Enable reverb
        static constexpr const SigByte tell_lies_on_the_internet[] = { 0x66, 0xB8, 0x0D, 0x00 };
        write_code_s(first_person_reverb_1, tell_lies_on_the_internet);

        // Do the thing
        static Hook hook;
        write_jmp_call(reinterpret_cast<std::byte *>(first_person_reverb_2), hook, nullptr, reinterpret_cast<const void *>(override_fp_reverb_position_asm), false);
    }

    void disable_fp_reverb_fix() noexcept {
        auto &chimera = get_chimera();
        chimera.get_signature("first_person_reverb_1_sig").rollback();
        chimera.get_signature("first_person_reverb_2_sig").rollback();
    }
}
