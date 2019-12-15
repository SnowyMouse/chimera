#include <optional>
#define _USE_MATH_DEFINES
#include <cmath>

#include "../../command.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../output/draw_text.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../console/console.hpp"
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
        void spectate_swap_eax_object_id_asm() noexcept;
        void spectate_swap_edx_object_id_asm() noexcept;
        void spectate_swap_ecx_asm() noexcept;
        void spectate_swap_esi_asm() noexcept;

        BaseDynamicObject *spectate_object_addr_eax_asm() noexcept;
    }

    bool spectate_enabled = false;

    static unsigned long rcon_id_being_spectated;
    static PlayerID player_being_spectated;
    bool spectate_command(int argc, const char **argv);

    static void force_unset_everything() {
        remove_preframe_event(force_unset_everything);
        get_chimera().execute_command("chimera_spectate 0");
    }

    static std::byte *get_player_data() noexcept {
        static std::optional<std::byte *> player_data;
        if(!player_data.has_value()) {
            player_data = **reinterpret_cast<std::byte ***>(get_chimera().get_signature("spectate_fp_camera_position_sig").data() + 2);
        }
        return *player_data;
    }

    static ObjectID &get_object_id() noexcept {
        return *reinterpret_cast<ObjectID *>(get_player_data() + 0x10);
    }

    static void set_object_id(const ObjectID &object_id) noexcept {
        get_object_id() = object_id;
    }

    extern "C" BaseDynamicObject *spectate_get_player_object_addr() noexcept {
        auto &table = PlayerTable::get_player_table();
        auto *player = table.get_player(player_being_spectated);
        auto &otable = ObjectTable::get_object_table();
        if(player) {
            auto &object_id = player->object_id;
            auto *object = otable.get_dynamic_object(object_id);
            if(object) {
                auto *parent = reinterpret_cast<UnitDynamicObject *>(otable.get_dynamic_object(object->parent));
                if(parent && (parent->type == ObjectType::OBJECT_TYPE_BIPED || parent->type == ObjectType::OBJECT_TYPE_VEHICLE) && parent->gunner == object_id) {
                    return parent;
                }
            }
            return object;
        }
        else {
            return nullptr;
        }
    }

    static void set_object_id_to_target() {
        if(server_type() == ServerType::SERVER_NONE) {
            add_preframe_event(force_unset_everything);
        }

        auto &table = PlayerTable::get_player_table();
        ObjectID id_to_set_to;
        auto *player = table.get_player(player_being_spectated);

        if(!spectate_enabled || !player) {
            player = table.get_client_player();
            add_preframe_event(force_unset_everything);
        }

        if(player) {
            id_to_set_to = player->object_id;

            // Show some text if the player doesn't have the console out
            if(!get_console_open()) {
                ColorARGB color;
                color.alpha = 0.8F;
                color.red = 1.0F;
                color.green = 1.0F;
                color.blue = 1.0F;

                if(id_to_set_to.is_null()) {
                    float seconds = static_cast<unsigned int>(player->respawn_time) / 30.0F;

                    char dead_text[256];
                    if(seconds > 0) {
                        std::snprintf(dead_text, sizeof(dead_text), "%.0f", seconds);
                    }
                    else {
                        std::snprintf(dead_text, sizeof(dead_text), "Waiting for space to clear");
                    }

                    // Change the alpha based on the respawn time
                    float ratio = seconds / 5.0F;
                    if(ratio > 1.0F) {
                        ratio = 1.0F;
                    }

                    color.alpha = 1.0F - ratio * 0.5F;

                    auto large_font = get_generic_font(GenericFont::FONT_LARGE);
                    apply_text(std::string(dead_text), -320, 60, 640, 480, color, large_font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);

                    auto old_object_id = player->last_object_id;
                    auto *old_object = ObjectTable::get_object_table().get_dynamic_object(old_object_id);
                    if(old_object && old_object->type == ObjectType::OBJECT_TYPE_BIPED) {
                        id_to_set_to = old_object_id;
                    }
                }

                color.alpha = 0.8F;

                auto small_font = get_generic_font(GenericFont::FONT_SMALL);
                auto height = font_pixel_height(small_font);
                apply_text(std::wstring(player->name), -320, 240 - height * 2, 640, 480, color, small_font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);
            }
        }
        else {
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
            return table.get_client_player()->get_full_id().whole_id;
        }
    }

    extern "C" std::uint32_t spectate_get_object_id() noexcept {
        auto &table = PlayerTable::get_player_table();
        auto *player = table.get_player(player_being_spectated);
        if(player) {
            return player->object_id.whole_id;
        }
        else {
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
        try {
            rcon_id_being_spectated = std::stoul(*argv);
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
        auto &spectate_hud_sig = get_chimera().get_signature("spectate_hud_sig");
        auto &spectate_grenade_hud_sig = get_chimera().get_signature("spectate_grenade_hud_sig");
        auto &spectate_health_hud_sig = get_chimera().get_signature("spectate_health_hud_sig");

        // If index is 0, disable
        if(rcon_id_being_spectated == 0) {
            console_output("off");
            if(spectate_enabled) {
                spectate_armor_color_sig.rollback();
                spectate_motion_sensor_sig.rollback();
                spectate_fp_animation_1_sig.rollback();
                spectate_fp_animation_2_sig.rollback();
                spectate_fp_weapon_sig.rollback();
                spectate_fp_hide_player_sig.rollback();
                spectate_reticle_team_sig.rollback();
                spectate_hud_sig.rollback();
                spectate_grenade_hud_sig.rollback();
                spectate_health_hud_sig.rollback();

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
        Player *player = player_table.get_player_by_rcon_id(rcon_id_being_spectated - 1);
        if(player) {
            player_being_spectated = player->get_full_id();

            static constexpr SigByte nop3[] = { 0x90, 0x90, 0x90 };
            static constexpr SigByte nop4[] = { 0x90, 0x90, 0x90, 0x90 };
            static constexpr SigByte nop5[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
            static constexpr SigByte nop7[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

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

                static Hook spectate_hud;
                auto *spectate_hud_data = spectate_hud_sig.data();
                write_code_s(spectate_hud_data + 0x08, nop3);
                write_code_s(spectate_hud_data + 0x15, nop5);
                write_code_s(spectate_hud_data + 0x1A, nop3);
                write_code_s(spectate_hud_data + 0x1D, nop4);
                write_jmp_call(spectate_hud_data + 0x15, spectate_hud, reinterpret_cast<const void *>(spectate_object_addr_eax_asm), nullptr, false);

                static Hook spectate_grenade_hud;
                auto *spectate_grenade_hud_data = spectate_grenade_hud_sig.data();
                write_jmp_call(spectate_grenade_hud_data, spectate_grenade_hud, reinterpret_cast<const void *>(spectate_swap_eax_object_id_asm), nullptr, false);

                static Hook spectate_health_hud;
                auto *spectate_health_hud_data = spectate_health_hud_sig.data();
                write_jmp_call(spectate_health_hud_data, spectate_health_hud, reinterpret_cast<const void *>(spectate_swap_edx_object_id_asm), nullptr, false);
            }

            spectate_enabled = true;

            console_output(localize("chimera_spectate_command_now_spectating"), player->name);
        }
        else {
            console_error(localize("chimera_error_player_not_found"), *argv);
        }

        return true;
    }

    static bool team_only = false;

    bool cycle_spectate(int increment) {
        auto &player_table = PlayerTable::get_player_table();
        if(!spectate_enabled) {
            rcon_id_being_spectated = 1;
        }
        unsigned long i = rcon_id_being_spectated + increment;
        std::optional<std::uint8_t> team_to_ignore = team_only && is_team() ? std::optional<std::uint8_t>(player_table.get_client_player()->team) : std::nullopt;
        while(true) {
            if(increment > 0) {
                if(i >= UINT8_MAX) {
                    i = 1;
                }
            }
            else {
                if(i == 0) {
                    i = UINT8_MAX - 1;
                }
            }
            Player *player = player_table.get_player_by_rcon_id(i - 1);
            if(i == rcon_id_being_spectated && (!player || spectate_enabled)) {
                console_error(localize("chimera_spectate_next_command_nobody_to_spectate"));
                return false;
            }
            if(player && (!team_to_ignore.has_value() || player->team == team_to_ignore)) {
                char arg[256];
                const char *arg_ptr = arg;
                std::snprintf(arg, sizeof(arg), "%lu", i);
                return spectate_command(1, &arg_ptr);
            }
            i += increment;
        }
    }

    bool spectate_next_command(int, const char **) {
        return cycle_spectate(1);
    }

    bool spectate_previous_command(int, const char **) {
        return cycle_spectate(-1);
    }

    bool spectate_team_only_command(int argc, const char **argv) {
        if(argc == 1) {
            team_only = STR_TO_BOOL(argv[0]);
        }
        console_output(BOOL_TO_STR(team_only));
        return true;
    }
}
