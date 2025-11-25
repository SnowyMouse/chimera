// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <thread>

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    static double max_spf;
    static LARGE_INTEGER next_frame;
    static LARGE_INTEGER current_frame;
    static double pc_freq;
    static double duration_to_add;
    static bool limiter_enabled = false;

    static void on_frame() {
        if(limiter_enabled) {
            QueryPerformanceCounter(&current_frame);
            while(static_cast<long long>(current_frame.QuadPart) < static_cast<long long>(next_frame.QuadPart)) {
                Sleep(0);
                QueryPerformanceCounter(&current_frame);
            }
            next_frame.QuadPart = current_frame.QuadPart + static_cast<long long>(duration_to_add);
        }
    }

    bool throttle_fps_command(int argument_count, const char **arguments) noexcept {
        static bool enabled = false;
        static bool hook_enabled = false;

        if(argument_count) {
            float new_fps = std::strtof(arguments[0], nullptr);

            // If user inputs an invalid framerate, assume they are turning it off.
            if(new_fps <= 0) {
                enabled = false;
                limiter_enabled = false;
            }
            else {
                // Assume 1 means you want the default.
                if(new_fps == 1.0f) {
                    new_fps = 300.0f;
                }
                enabled = true;
                max_spf = 1.0f / new_fps;
                QueryPerformanceFrequency(&current_frame);
                pc_freq = static_cast<double>(current_frame.QuadPart);
                duration_to_add = max_spf * pc_freq;
                if(!hook_enabled) {
                    static Hook hook;
                    write_jmp_call(get_chimera().get_signature("d3d9_present_frame_sig").data() + 3, hook, reinterpret_cast<const void*>(on_frame), nullptr);
                    hook_enabled = true;
                }
                limiter_enabled = true;
            }
        }

        if(enabled) {
            console_output("%.02f FPS", 1.0 / max_spf);
        }
        else {
            console_output("off");
        }

        return true;
    }
}
