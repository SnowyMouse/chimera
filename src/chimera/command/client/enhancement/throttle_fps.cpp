// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <chrono>
#include <thread>

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../event/frame.hpp"

namespace Chimera {
    static float max_spf;
    typedef std::chrono::high_resolution_clock clock;
    static clock::time_point next_frame;

    static void on_frame() {
        while(clock::now() < next_frame) {
            Sleep(0);
        }
        auto duration_to_add = std::chrono::duration<long long, std::micro>(static_cast<long long>(max_spf * 1000000));
        long long multiplier = 1 + static_cast<long long>((clock::now() - next_frame) / duration_to_add);
        next_frame += duration_to_add * multiplier;
    }

    bool throttle_fps_command(int argument_count, const char **arguments) noexcept {
        static bool enabled = false;

        if(argument_count) {
            float new_fps = std::strtof(arguments[0], nullptr);

            // If user inputs an invalid framerate, assume they are turning it off.
            if(new_fps <= 0) {
                enabled = false;
                get_chimera().get_signature("d3d9_present_frame_sig").rollback();
            }
            else {
                enabled = true;
                max_spf = 1.0f / new_fps;
                next_frame = clock::now();
                static Hook hook;
                write_jmp_call(get_chimera().get_signature("d3d9_present_frame_sig").data() + 3, hook, reinterpret_cast<const void*>(on_frame), nullptr);
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
