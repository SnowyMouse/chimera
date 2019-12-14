#include <optional>

#include "../../command.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../event/frame.hpp"

namespace Chimera {
    extern "C" {
        std::uint32_t spectate_swap_eax_asm() noexcept;
        void spectate_swap_ecx_asm() noexcept;
        void spectate_swap_ebx_object_id_asm() noexcept;
        void spectate_swap_esi_asm() noexcept;
    }

    static PlayerID player_being_spectated;
    bool spectate_command(int argc, const char **argv);

    static void force_unset_everything() {
        remove_preframe_event(force_unset_everything);
        get_chimera().execute_command("chimera_spectate 0");
    }

    static void set_object_id(const ObjectID &object_id) {
        static std::optional<ObjectID *> object_id_addr;
        if(!object_id_addr.has_value()) {
            object_id_addr = reinterpret_cast<ObjectID *>(**reinterpret_cast<std::byte ***>(get_chimera().get_signature("spectate_fp_camera_position_sig").data() + 2) + 0x10);
        }
        **object_id_addr = object_id;
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

    bool spectate_command(int, const char **argv) {
        // Get the index maybe
        int index;
        static bool enabled = false;
        try {
            index = std::stoi(*argv);
        }
        catch(std::exception &) {
            console_error("Invalid player index");
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
            if(enabled) {
                spectate_armor_color_sig.rollback();
                spectate_motion_sensor_sig.rollback();
                spectate_fp_animation_1_sig.rollback();
                spectate_fp_animation_2_sig.rollback();
                spectate_fp_weapon_sig.rollback();
                spectate_fp_hide_player_sig.rollback();
                spectate_reticle_team_sig.rollback();

                remove_preframe_event(set_object_id_to_target);
                auto *player = PlayerTable::get_player_table().get_client_player();
                ObjectID id_to_set_to;
                if(player) {
                    id_to_set_to = player->object_id;
                }
                else {
                    id_to_set_to.whole_id = 0xFFFFFFFF;
                }
                set_object_id(id_to_set_to);

                enabled = false;
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

            if(!enabled) {
                add_preframe_event(set_object_id_to_target);

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

            enabled = true;

            console_output("Spectating %S", player->name);
        }
        else {
            console_error("Player not present");
        }

        return true;
    }
}
