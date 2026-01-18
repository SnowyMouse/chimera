// SPDX-License-Identifier: GPL-3.0-only

#include "../../../event/camera.hpp"
#include "../../../halo_data/camera.hpp"
#include "../../../halo_data/resolution.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../fix/fp_model.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>
#include <optional>

#define DEG_TO_RAD(deg) (deg * static_cast<float>(M_PI) / 180.0f)
#define RAD_TO_DEG(rad) (rad / static_cast<float>(M_PI) * 180.0f)

namespace Chimera {
    static float previous_value;

    static std::optional<float> setting_first_person;
    static std::optional<float> setting_vehicle;
    static std::optional<float> setting_cinematic;

    static bool vertical_first_person;
    static bool vertical_vehicle;
    static bool vertical_cinematic;

    static void change_camera() noexcept {
        auto &camera = camera_data();
        previous_value = camera.fov;

        // Get the FoV to use
        auto fov_to_use = setting_first_person;
        bool vertical = vertical_first_person;
        bool use_unit_fov = false;
        switch(camera_type()) {
            case CameraType::CAMERA_FIRST_PERSON:
                fov_to_use = setting_first_person;
                vertical = vertical_first_person;
                use_unit_fov = true;
                break;
            case CameraType::CAMERA_VEHICLE:
                fov_to_use = setting_vehicle;
                vertical = vertical_vehicle;
                break;
            default:
                fov_to_use = setting_cinematic;
                vertical = vertical_cinematic;
                break;
        }

        // If it's not set, default to first person
        if(!fov_to_use.has_value()) {
            fov_to_use = setting_first_person;
            vertical = vertical_first_person;
        }

        // And if that doesn't work, give up
        if(!fov_to_use.has_value()) {
            return;
        }

        // Get the multiplier.
        constexpr static float BASE_FOV = DEG_TO_RAD(70.0f);
        constexpr static float MAX_UNIT_FOV = DEG_TO_RAD(90.0f);
        float fov_multiplier = camera.fov / BASE_FOV;
        auto *player = PlayerTable::get_player_table().get_client_player();
        if(player && use_unit_fov) {
            auto &object_table = ObjectTable::get_object_table();
            auto *player_object = reinterpret_cast<UnitDynamicObject *>(object_table.get_dynamic_object(player->object_id));
            if(player_object) {
                auto *player_object_tag = get_tag(player_object->tag_id);
                float unit_field_of_view = *reinterpret_cast<float *>(player_object_tag->data + 0x17C + 0x24); // unit camera_field_of_view
                // The game does this somewhere so we have to too.
                if(unit_field_of_view > MAX_UNIT_FOV && player_object->zoom_level == 255) {
                    unit_field_of_view = MAX_UNIT_FOV;
                }
                fov_multiplier = camera.fov / unit_field_of_view;
            }
        }

        float setting = fov_to_use.value();

        if(vertical) {
            // Convert to a vertical FOV based on the resolution of the game.
            auto &resolution = get_resolution();
            camera.fov = 2.0f * std::atan(std::tan(setting / 2.0f) * resolution.width / resolution.height) * fov_multiplier;
        }
        else {
            // If we're just changing horizontal FOV, we don't need to do much math.
            camera.fov = fov_multiplier * setting;
        }
    }

    static void unchange_camera() noexcept {
        camera_data().fov = previous_value;
    }

    static bool fov_command_action(int argc, const char **args, std::optional<float> &setting, bool &vertical) {
        if(argc) {
            // Automatic FoV
            if(std::strcmp(args[0], "auto") == 0) {
                setting = 2.0f * static_cast<float>(std::atan(static_cast<float>(std::tan(DEG_TO_RAD(70.0f) / 2.0f)) * 3.0f / 4.0f));
                vertical = true;
            }

            // Off?
            else if(std::strcmp(args[0], "off") == 0) {
                setting = std::nullopt;
            }

            // We know what we want
            else {
                std::size_t end = 0;
                try {
                    setting = DEG_TO_RAD(static_cast<float>(std::stof(args[0], &end)));
                }
                catch(std::exception &) {
                    console_error(localize("chimera_fov_error_invalid_fov_given"));
                    return false;
                }
                vertical = (end && args[0][end] == 'v');
            }

            // Unregister everything
            remove_precamera_event(change_camera);
            remove_camera_event(unchange_camera);

            // Re-register everything
            if(setting_first_person.has_value() || setting_vehicle.has_value() || setting_cinematic.has_value()) {
                add_precamera_event(change_camera);
                add_camera_event(unchange_camera);
            }
        }

        // Display the current setting.
        if(setting.has_value()) {
            console_output("%.02f\xB0 %s", RAD_TO_DEG(*setting), vertical ? "(vertical)" : "(horizontal)");
        }
        else {
            console_output("off");
        }

        return true;
    }

    bool fov_command(int argc, const char **args) noexcept {
        return fov_command_action(argc, args, setting_first_person, vertical_first_person);
    }

    bool fov_vehicle_command(int argc, const char **args) noexcept {
        return fov_command_action(argc, args, setting_vehicle, vertical_vehicle);
    }

    bool fov_cinematic_command(int argc, const char **args) noexcept {
        return fov_command_action(argc, args, setting_cinematic, vertical_cinematic);
    }

    bool fov_fp_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(!new_enabled) {
                    lock_fp_model_fov = false;
                }
                else {
                    lock_fp_model_fov = true;
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
