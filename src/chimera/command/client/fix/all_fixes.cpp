// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"

namespace Chimera {
    template<void (*fix)(), const char * const unfix_sig> static bool fix_command(int argc, const char ** argv) {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                if(new_value) {
                    fix();
                }
                else {
                    get_chimera().get_signature(unfix_sig).rollback();
                }
                active = new_value;
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
    
    template<void (*fix)(), void (*unfix)()> static bool fix_command(int argc, const char ** argv) noexcept {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                if(new_value) {
                    fix();
                }
                else {
                    unfix();
                }
                active = new_value;
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
    
    #define DEFINE_FIX_UNFIX_SIG(command, fix, sig) char sig[] = #sig; void fix() noexcept; bool command(int argc, const char **argv) { return fix_command<fix, sig>(argc, argv); }
    #define DEFINE_FIX_UNFIX_FN(command, fix, unfix) void fix() noexcept; void unfix() noexcept; bool command(int argc, const char **argv) { return fix_command<fix, unfix>(argc, argv); }
    
    DEFINE_FIX_UNFIX_SIG(fix_motion_sensor, set_up_motion_sensor_fix, motion_sensor_update_sig)
    DEFINE_FIX_UNFIX_SIG(fix_lens_flare_scaling, set_up_sun_fix, lens_scale_sig)
    DEFINE_FIX_UNFIX_SIG(fix_contrails, set_up_contrail_fix, contrail_update_sig)
    DEFINE_FIX_UNFIX_SIG(fix_blue_particles, set_up_blue_32bit_color_fix, blue_32bit_color_sig)
    DEFINE_FIX_UNFIX_SIG(fix_flashlight, set_up_flashlight_fix, flashlight_radius_sig)
    DEFINE_FIX_UNFIX_FN(fix_inverted_flags, set_up_inverted_flag_fix, unset_up_inverted_flag_fix)
}
