// SPDX-License-Identifier: GPL-3.0-only

#include "../../../event/frame.hpp"
#include "../../../output/draw_text.hpp"
#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include <windows.h>

namespace Chimera {
    static LARGE_INTEGER last_frame;

    static double last_fps;
    static double pc_freq;

    static int total_frames;
    static double total_frame_time;

    static void show_fps() noexcept;
    bool show_fps_command(int argc, const char **argv) noexcept {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(new_enabled) {
                    LARGE_INTEGER freq;
                    QueryPerformanceFrequency(&freq);
                    pc_freq = static_cast<double>(freq.QuadPart);
                    add_preframe_event(show_fps, EventPriority::EVENT_PRIORITY_FINAL);
                }
                else {
                    remove_preframe_event(show_fps);
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));
        return true;
    }

    static void show_fps() noexcept {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        // Add the current frame time to the total
        auto difference = now.QuadPart - last_frame.QuadPart;
        total_frames++;

        auto frame_time = difference / pc_freq;
        total_frame_time += frame_time;

        // If we've past 1/4 of a second, show the new FPS
        if(total_frame_time > (1.0 / 4.0)) {
            last_fps = static_cast<double>(total_frames) / total_frame_time;
            total_frames = 0;
            total_frame_time = 0.0;
        }

        // Display average FPS (if available)
        char avg[64] = "-- AVG";
        if(last_fps > 0.0) {
            std::snprintf(avg, sizeof(avg), "%.00f", last_fps);
        }
        static ColorARGB blue = ColorARGB { 0.7, 0.45, 0.72, 1.0 };
        apply_text(std::move(avg), 100, 0, 100, font_pixel_height(GenericFont::FONT_CONSOLE), blue, GenericFont::FONT_CONSOLE, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_TOP_RIGHT);

        last_frame = now;
    }
}
