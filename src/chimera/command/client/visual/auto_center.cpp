#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../event/tick.hpp"
#include "../../../event/frame.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"

namespace Chimera {
    static void apply_mod() noexcept;
    static void auto_center_frame() noexcept;
    static void auto_center_tick() noexcept;

    bool auto_center_command(int argc, const char **argv) {
        static int auto_center = 0;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            auto &auto_center_s = get_chimera().get_signature("auto_center_sig");
            auto new_value = atol(argv[0]);
            if(new_value != auto_center) {
                auto_center_s.rollback();
                remove_pretick_event(auto_center_tick);
                remove_frame_event(auto_center_frame);

                switch(new_value) {
                case 0:
                    break;
                case 1:
                    add_pretick_event(auto_center_tick);
                    break;
                case 2:
                    apply_mod();
                    break;
                default:
                    console_error(localize("chimera_auto_center_command_invalid_setting"));
                    return false;
                }

                auto_center = new_value;
            }
        }

        console_output("%i", auto_center);
        return true;
    }

    // Apply the mod, disabling auto centering.
    static void apply_mod() noexcept {
        static const short mod[] = { 0x8B, 0x51, 0x54, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x90, 0x90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x90, 0xE9 };
        static auto *auto_center_ptr = get_chimera().get_signature("auto_center_sig").data();
        write_code(auto_center_ptr, mod, sizeof(mod)/sizeof(mod[0]));
    }

    // This is the number of frames that occurred this tick.
    static std::size_t frames = 0;

    // Disable auto centering once a second frame occurs.
    static void auto_center_frame() noexcept {
        if(++frames == 2) {
            apply_mod();
            remove_frame_event(auto_center_frame);
        }
    }

    // Re-enable auto centering, ensuring that the camera movement only occurs only occurs once per tick. Set frame counter to 0.
    static void auto_center_tick() noexcept {
        get_chimera().get_signature("auto_center_sig").rollback();
        frames = 0;
        add_frame_event(auto_center_frame);
    }
}
