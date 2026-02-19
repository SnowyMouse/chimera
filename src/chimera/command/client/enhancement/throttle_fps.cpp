// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <thread>

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    static LARGE_INTEGER pc_freq;
    static LARGE_INTEGER current_frame;
    static LARGE_INTEGER last_frame;
    static long long frame_time_ticks;
    static bool limiter_enabled = false;

    void on_frame() noexcept {
        if(limiter_enabled) {
            bool slept_this_frame = false;
            QueryPerformanceCounter(&current_frame);
            long long time_since_last_frame = current_frame.QuadPart - last_frame.QuadPart;
            // I suppose this could technically happen...
            if(time_since_last_frame < 0) {
                return;
            }

            while(time_since_last_frame < frame_time_ticks) {
                // Sleep for a chunk of the remaining time slice...
                if(time_since_last_frame < (frame_time_ticks / 2) && !slept_this_frame) {
                    Sleep(((time_since_last_frame * 1000) / pc_freq.QuadPart) / 2);
                    slept_this_frame = true;
                }
                else if(time_since_last_frame < frame_time_ticks * 0.75) {
                    Sleep(0);
                }
                // And then just spin the while loop for the rest.
                QueryPerformanceCounter(&current_frame);
                time_since_last_frame = current_frame.QuadPart - last_frame.QuadPart;
            }
            last_frame.QuadPart = current_frame.QuadPart;
        }
    }

    bool throttle_fps_command(int argument_count, const char **arguments) noexcept {
        static bool enabled = false;
        static bool hook_enabled = false;
        static float frame_time_target = 0;

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
                QueryPerformanceFrequency(&current_frame);
                pc_freq.QuadPart = current_frame.QuadPart;
                frame_time_target = 1.0f / new_fps;
                frame_time_ticks = pc_freq.QuadPart / static_cast<long>(new_fps);
                QueryPerformanceCounter(&current_frame);
                last_frame.QuadPart = 0;

                if(!hook_enabled) {
                    static Hook hook;
                    write_jmp_call(get_chimera().get_signature("d3d9_present_frame_sig").data(), hook, reinterpret_cast<const void*>(on_frame), nullptr);
                    hook_enabled = true;
                }
                limiter_enabled = true;
            }
        }

        if(enabled) {
            console_output("%.02f FPS", 1.0 / frame_time_target);
        }
        else {
            console_output("off");
        }

        return true;
    }
}
