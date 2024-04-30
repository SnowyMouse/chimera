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
#include "../../event/revert.hpp"
#include "../../halo_data/game_engine.hpp"
#include "../../output/output.hpp"

#include "antenna.hpp"
#include "camera.hpp"
#include "contrail.hpp"
#include "flag.hpp"
#include "fp.hpp"
#include "light.hpp"
#include "object.hpp"
#include "particle.hpp"

#include "interpolate.hpp"

namespace Chimera {
    // This is the progress since the last tick (updated every frame).
    float interpolation_tick_progress = 0;

    // This is the assumed tick rate of the first person camera.
    static float *first_person_camera_tick_rate = nullptr;

    // Set for if interpolation is enabled
    bool interpolation_enabled = false;


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
        fix_contrail_on_tick();
        interpolation_tick_progress = 0;
        float current_tick_rate = effective_tick_rate();
        if(*first_person_camera_tick_rate != current_tick_rate) {
            overwrite(first_person_camera_tick_rate, current_tick_rate);
        }
    }

    static void on_preframe() noexcept {
        if(game_paused()) {
            return;
        }

        interpolation_tick_progress = get_tick_progress();

        interpolate_antenna_before();
        interpolate_flag_before();
        interpolate_light_before();
        interpolate_object_before();
        interpolate_particle();
    }

    static void on_frame() noexcept {
        if(game_paused()) {
            return;
        }

        interpolate_antenna_after();
        interpolate_object_after();
        interpolate_particle_after();
    }

    void clear_buffers() noexcept {
        interpolate_object_clear();
        interpolate_particle_clear();
        interpolate_light_clear();
        interpolate_flag_clear();
        interpolate_camera_clear();
        interpolate_fp_clear();
        fix_contrail_clear();
    }

    void set_up_interpolation() noexcept {
        static auto *fp_interp_ptr = get_chimera().get_signature("fp_interp_sig").data();
        static Hook fp_interp_hook;
        first_person_camera_tick_rate = *reinterpret_cast<float **>(get_chimera().get_signature("fp_cam_tick_rate_sig").data() + 2);
        static auto *contrail_update_ptr = get_chimera().get_signature("contrail_update_func_sig").data();
        static Hook contail_update_hook;

        add_tick_event(on_tick);
        add_preframe_event(on_preframe);
        add_frame_event(on_frame);
        add_precamera_event(interpolate_camera_before);
        add_camera_event(interpolate_camera_after);
        write_jmp_call(fp_interp_ptr, fp_interp_hook, reinterpret_cast<const void *>(interpolate_fp_before), reinterpret_cast<const void *>(interpolate_fp_after));

        // Make sure contrails trail their parent object when interpolating said objects.
        write_jmp_call(contrail_update_ptr, contail_update_hook, reinterpret_cast<const void *>(fix_contrail_before), reinterpret_cast<const void *>(fix_contrail_after));

        // Block built-in fp camera interpolation. Let Chimera do it instead.
        overwrite(get_chimera().get_signature("camera_interpolation_sig").data() + 0xF, static_cast<unsigned char>(0xEB));

        //Clear interpolation buffers on major game state changes to prevent funny things from happening
        add_revert_event(clear_buffers);
        interpolation_enabled = true;
    }

    void disable_interpolation() noexcept {
        get_chimera().get_signature("fp_interp_sig").rollback();
        get_chimera().get_signature("camera_interpolation_sig").rollback();
        get_chimera().get_signature("contrail_update_func_sig").rollback();
        remove_tick_event(on_tick);
        remove_preframe_event(on_preframe);
        remove_frame_event(on_frame);
        remove_precamera_event(interpolate_camera_before);
        remove_camera_event(interpolate_camera_after);
        remove_revert_event(clear_buffers);
        interpolation_enabled = false;
    }
}
