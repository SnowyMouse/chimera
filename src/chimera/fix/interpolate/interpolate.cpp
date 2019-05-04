#include <cctype>

#include "../../chimera.hpp"
#include "../../signature/hook.hpp"
#include "../../signature/signature.hpp"
#include "../../halo_data/multiplayer.hpp"
#include "../../event/camera.hpp"
#include "../../event/frame.hpp"
#include "../../event/tick.hpp"

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

    // This is the current effective tick rate.
    static float interpolation_current_tick_rate = 30.0;

    static void on_tick() noexcept {
        interpolate_antenna_on_tick();
        interpolate_flag_on_tick();
        interpolate_fp_on_tick();
        interpolate_light_on_tick();
        interpolate_object_on_tick();
        interpolate_camera_on_tick();
        interpolate_particle_on_tick();
        interpolation_tick_progress = 0;
        interpolation_current_tick_rate = effective_tick_rate();
        if(*first_person_camera_tick_rate != interpolation_current_tick_rate) {
            overwrite(first_person_camera_tick_rate, interpolation_current_tick_rate);
        }
    }

    static void on_preframe() noexcept {
        interpolation_tick_progress = interpolation_current_tick_rate * *reinterpret_cast<float *>(0x40000304);

        if(interpolation_tick_progress > 1) {
            interpolation_tick_progress = 1;
        }

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
    }

    void disable_interpolation() noexcept {
        get_chimera().get_signature("fp_interp_sig").rollback();
        remove_tick_event(on_tick);
        remove_preframe_event(on_preframe);
        remove_frame_event(on_frame);
        remove_precamera_event(interpolate_camera_before);
        remove_camera_event(interpolate_camera_after);
    }
}
