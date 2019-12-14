#include <optional>
#define _USE_MATH_DEFINES
#include <cmath>

#include "../../command.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../halo_data/camera.hpp"
#include "../../../event/camera.hpp"
#include "../../../math_trig/math_trig.hpp"
#include "../../../event/frame.hpp"

namespace Chimera {
    extern "C" {
        std::uint32_t spectate_swap_eax_asm() noexcept;
        void spectate_swap_ecx_asm() noexcept;
        void spectate_swap_ebx_object_id_asm() noexcept;
        void spectate_swap_esi_asm() noexcept;
    }

    bool spectate_enabled = false;

    static PlayerID player_being_spectated;
    bool spectate_command(int argc, const char **argv);

    static void force_unset_everything() {
        remove_preframe_event(force_unset_everything);
        get_chimera().execute_command("chimera_spectate 0");
    }

    static std::byte *get_player_data() {
        static std::optional<std::byte *> player_data;
        if(!player_data.has_value()) {
            player_data = **reinterpret_cast<std::byte ***>(get_chimera().get_signature("spectate_fp_camera_position_sig").data() + 2);
        }
        return *player_data;
    }

    static void set_object_id(const ObjectID &object_id) {
        *reinterpret_cast<ObjectID *>(get_player_data() + 0x10) = object_id;
    }

    static void set_object_id_to_target() {
        auto &table = PlayerTable::get_player_table();
        ObjectID id_to_set_to;
        auto *player = table.get_player(player_being_spectated);
        if(player) {
            id_to_set_to = player->object_id;
        }
        else {
            add_preframe_event(force_unset_everything);
            id_to_set_to.whole_id = 0xFFFFFFFF;
        }
        set_object_id(id_to_set_to);
    }

    extern "C" std::uint32_t spectate_get_player_id() noexcept {
        auto &table = PlayerTable::get_player_table();
        if(table.get_player(player_being_spectated)) {
            return player_being_spectated.whole_id;
        }
        else {
            add_preframe_event(force_unset_everything);
            return 0xFFFFFFFF;
        }
    }

    static void on_precamera() noexcept {
        auto *player = PlayerTable::get_player_table().get_player(player_being_spectated);
        if(!player || player->object_id.is_null()) {
            return;
        }

        auto *object = reinterpret_cast<UnitDynamicObject *>(ObjectTable::get_object_table().get_dynamic_object(player->object_id));
        if(!object) {
            return;
        }

        float *xyz = reinterpret_cast<float *>(get_player_data() + 0x1C);
        float pitch = std::asin(object->aim.z);
        float cos_pitch = std::cos(pitch);
        float yaw = M_PI / 2 - std::asin(object->aim.x / cos_pitch);
        if(object->aim.y < 0.0F) {
            yaw *= -1.0F;
        }

        xyz[0] = yaw;
        xyz[1] = pitch;
    }

    bool spectate_command(int, const char **argv) {
        // Get the index maybe
        int index;
        try {
            index = std::stoul(*argv);
        }
        catch(std::exception &) {
            console_error(localize("chimera_error_takes_player_number"));
            return false;
        }

        // Hooks and stuff
        auto &spectate_armor_color_sig = get_chimera().get_signature("spectate_armor_color_sig");
        auto &spectate_motion_sensor_sig = get_chimera().get_signature("spectate_motion_sensor_sig");
        auto &spectate_fp_animation_1_sig = get_chimera().get_signature("spectate_fp_animation_1_sig");
        auto &spectate_fp_animation_2_sig = get_chimera().get_signature("spectate_fp_animation_2_sig");
        auto &spectate_fp_weapon_sig = get_chimera().get_signature("spectate_fp_weapon_sig");
        auto &spectate_fp_hide_player_sig = get_chimera().get_signature("spectate_fp_hide_player_sig");
        auto &spectate_reticle_team_sig = get_chimera().get_signature("spectate_reticle_team_sig");

        // If index is 0, disable
        if(index == 0) {
            console_output("off");
            if(spectate_enabled) {
                spectate_armor_color_sig.rollback();
                spectate_motion_sensor_sig.rollback();
                spectate_fp_animation_1_sig.rollback();
                spectate_fp_animation_2_sig.rollback();
                spectate_fp_weapon_sig.rollback();
                spectate_fp_hide_player_sig.rollback();
                spectate_reticle_team_sig.rollback();

                remove_preframe_event(set_object_id_to_target);
                remove_precamera_event(on_precamera);
                auto *player = PlayerTable::get_player_table().get_client_player();
                ObjectID id_to_set_to;
                if(player) {
                    id_to_set_to = player->object_id;
                }
                else {
                    id_to_set_to.whole_id = 0xFFFFFFFF;
                }
                set_object_id(id_to_set_to);

                spectate_enabled = false;
            }
            return true;
        }

        // Get the player if needed
        auto &player_table = PlayerTable::get_player_table();
        Player *player = player_table.get_player_by_rcon_id(index - 1);
        if(player) {
            player_being_spectated = player->get_full_id();

            static constexpr SigByte nop3[] = { 0x90, 0x90, 0x90 };
            static constexpr SigByte nop4[] = { 0x90, 0x90, 0x90, 0x90 };
            static constexpr SigByte nop7[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
            //static constexpr SigByte nop7[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

            if(!spectate_enabled) {
                add_preframe_event(set_object_id_to_target);
                add_precamera_event(on_precamera, EVENT_PRIORITY_BEFORE);

                static Hook spectate_reticle_team;
                auto *spectate_reticle_team_data = spectate_reticle_team_sig.data();
                write_code_s(spectate_reticle_team_data, nop7);
                write_jmp_call(spectate_reticle_team_data, spectate_reticle_team, reinterpret_cast<const void *>(spectate_swap_eax_asm), nullptr, false);

                static Hook spectate_fp_hide_player;
                auto *spectate_fp_hide_player_data = spectate_fp_hide_player_sig.data();
                write_code_s(spectate_fp_hide_player_data, nop4);
                write_jmp_call(spectate_fp_hide_player_data, spectate_fp_hide_player, reinterpret_cast<const void *>(spectate_swap_eax_asm), nullptr, false);

                static Hook spectate_armor_color;
                auto *armor_color_data = spectate_armor_color_sig.data();
                write_code_s(armor_color_data, nop4);
                write_jmp_call(armor_color_data, spectate_armor_color, reinterpret_cast<const void *>(spectate_swap_eax_asm), nullptr, false);

                static Hook spectate_motion_sensor;
                auto *motion_sensor_data = spectate_motion_sensor_sig.data();
                write_code_s(motion_sensor_data, nop4);
                write_jmp_call(motion_sensor_data, spectate_motion_sensor, reinterpret_cast<const void *>(spectate_swap_eax_asm), nullptr, false);

                static Hook spectate_fp_animation_1;
                auto *spectate_fp_animation_1_data = spectate_fp_animation_1_sig.data();
                write_code_s(spectate_fp_animation_1_data, nop4);
                write_jmp_call(spectate_fp_animation_1_data, spectate_fp_animation_1, reinterpret_cast<const void *>(spectate_swap_ecx_asm), nullptr, false);

                static Hook spectate_fp_animation_2;
                auto *spectate_fp_animation_2_data = spectate_fp_animation_2_sig.data();
                write_code_s(spectate_fp_animation_2_data, nop4);
                write_jmp_call(spectate_fp_animation_2_data, spectate_fp_animation_2, reinterpret_cast<const void *>(spectate_swap_ecx_asm), nullptr, false);

                static Hook spectate_fp_weapon;
                auto *spectate_fp_weapon_data = spectate_fp_weapon_sig.data();
                write_code_s(spectate_fp_weapon_data, nop3);
                write_jmp_call(spectate_fp_weapon_data, spectate_fp_weapon, reinterpret_cast<const void *>(spectate_swap_eax_asm), nullptr, false);
            }

            spectate_enabled = true;

            console_output(localize("chimera_spectate_command_now_spectating"), player->name);
        }
        else {
            console_error(localize("chimera_error_player_not_found"), *argv);
        }

        return true;
    }
}
