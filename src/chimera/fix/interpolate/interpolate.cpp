// SPDX-License-Identifier: GPL-3.0-only

#include <cctype>

#include "../../chimera.hpp"
#include "../../signature/hook.hpp"
#include "../../signature/signature.hpp"
#include "../../halo_data/multiplayer.hpp"
#include "../../halo_data/pause.hpp"
#include "../../event/camera.hpp"
#include "../../event/frame.hpp"
#include "../../event/tick.hpp"
#include "../../halo_data/game_engine.hpp"
#include "../../output/output.hpp"

#include "antenna.hpp"
#include "camera.hpp"
#include "flag.hpp"
#include "fp.hpp"
#include "light.hpp"
#include "object.hpp"
#include "particle.hpp"

#include "interpolate.hpp"

namespace Chimera {
    // This is the progress since the last tick (updated every frame).
    float interpolation_tick_progress = 0;

    // This will be called every frame to update the nav point position.
    //static void(*nav_point)() = nullptr;

    // This is the assumed tick rate of the first person camera.
    static float *first_person_camera_tick_rate = nullptr;

    // Set for if interpolation is enabled
    bool interpolation_enabled = false;

    // The last value of the tick count
    std::int32_t previous_tick = 0;


    static void on_tick() noexcept {
        // Prevent interpolation when the game is paused
        if(game_paused()) {
            return;
        }

        interpolate_antenna_on_tick();
        interpolate_flag_on_tick();
        interpolate_fp_on_tick();
        interpolate_light_on_tick();
        interpolate_object_on_tick();
        interpolate_camera_on_tick();
        interpolate_particle_on_tick();
        interpolation_tick_progress = 0;
        float current_tick_rate = effective_tick_rate();
        if(*first_person_camera_tick_rate != current_tick_rate) {
            overwrite(first_person_camera_tick_rate, current_tick_rate);
        }

        previous_tick = get_tick_count();
    }

    static void on_preframe() noexcept {
        if(game_paused()) {
            return;
        }

        // Check if we've reverted and if so, clear interpolation buffers.
        if (previous_tick > get_tick_count()) {
            interpolate_object_clear();
            interpolate_particle_clear();
            interpolate_light_clear();
            interpolate_flag_clear();
            interpolate_camera_clear();
            interpolate_fp_clear();
        }

        interpolation_tick_progress = get_tick_progress();

        interpolate_antenna_before();
        interpolate_flag_before();
        interpolate_light_before();
        interpolate_object_before();
        interpolate_particle();
        if(server_type() != ServerType::SERVER_NONE) {
            //nav_point();
        }
    }

    static void on_frame() noexcept {
        if(game_paused()) {
            return;
        }

        interpolate_antenna_after();
        interpolate_object_after();
        interpolate_particle_after();
    }

    void set_up_interpolation() noexcept {
        static auto *fp_interp_ptr = get_chimera().get_signature("fp_interp_sig").data();
        static Hook fp_interp_hook;
        first_person_camera_tick_rate = *reinterpret_cast<float **>(get_chimera().get_signature("fp_cam_tick_rate_sig").data() + 2);
        //nav_point = reinterpret_cast<void(*)()>(get_chimera().get_signature("nav_point_sig").data());

        add_tick_event(on_tick);
        add_preframe_event(on_preframe);
        add_frame_event(on_frame);
        add_precamera_event(interpolate_camera_before);
        add_camera_event(interpolate_camera_after);
        write_jmp_call(fp_interp_ptr, fp_interp_hook, reinterpret_cast<const void *>(interpolate_fp_before), reinterpret_cast<const void *>(interpolate_fp_after));
        interpolation_enabled = true;
    }

    void disable_interpolation() noexcept {
        get_chimera().get_signature("fp_interp_sig").rollback();
        remove_tick_event(on_tick);
        remove_preframe_event(on_preframe);
        remove_frame_event(on_frame);
        remove_precamera_event(interpolate_camera_before);
        remove_camera_event(interpolate_camera_after);
        interpolation_enabled = false;
    }
}
