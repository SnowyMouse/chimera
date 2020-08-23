// SPDX-License-Identifier: GPL-3.0-only

#include "../../../event/camera.hpp"
#include "../../../halo_data/camera.hpp"
#include "../../../halo_data/resolution.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"

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
        constexpr static float BASE_FOV = DEG_TO_RAD(70.0f);
        auto &camera = camera_data();
        float fov_multiplier = camera.fov / BASE_FOV;
        previous_value = camera.fov;

        // Get the FoV to use
        auto fov_to_use = setting_first_person;
        bool vertical = vertical_first_person;
        switch(camera_type()) {
            case CameraType::CAMERA_FIRST_PERSON:
                fov_to_use = setting_first_person;
                vertical = vertical_first_person;
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
}
