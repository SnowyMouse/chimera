#include "../../../event/camera.hpp"
#include "../../../halo_data/camera.hpp"
#include "../../../halo_data/resolution.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>

#define DEG_TO_RAD(deg) (deg * static_cast<float>(M_PI) / 180.0f)
#define RAD_TO_DEG(rad) (rad / static_cast<float>(M_PI) * 180.0f)

namespace Chimera {
    static float previous_value;
    static float setting;
    static bool vertical;

    static void change_camera() noexcept {
        constexpr static float BASE_FOV = DEG_TO_RAD(70.0f);
        auto &camera = camera_data();
        float fov_multiplier = camera.fov / BASE_FOV;
        previous_value = camera.fov;

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

    bool fov_command(int argc, const char **args) noexcept {
        static bool enabled = false;

        if(argc) {
            char *end = nullptr;

            if(std::strcmp(args[0], "auto") == 0) {
                setting = 2.0f * static_cast<float>(std::atan(static_cast<float>(std::tan(DEG_TO_RAD(70.0f) / 2.0f)) * 3.0f / 4.0f));
                vertical = true;
            }
            else {
                setting = DEG_TO_RAD(static_cast<float>(strtof(args[0], &end)));
                vertical = (end && *end == 'v');
            }

            // Remove all changes if disabling
            if(enabled && setting == 0) {
                remove_precamera_event(change_camera);
                remove_precamera_event(unchange_camera);
            }
            else if(!enabled && setting != 0) {
                add_precamera_event(change_camera);
                add_camera_event(unchange_camera);
            }
            enabled = (setting != 0);
        }

        // Display the current setting.
        if(enabled) {
            console_output("%.02f\xB0 %s", RAD_TO_DEG(setting), vertical ? "(vertical)" : "(horizontal)");
        }
        else {
            console_output("off");
        }

        return true;
    }
}
