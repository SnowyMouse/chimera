// SPDX-License-Identifier: GPL-3.0-only

#include "../../../event/frame.hpp"
#include "../../../event/tick.hpp"
#include "../../../event/map_load.hpp"
#include "../../../output/draw_text.hpp"
#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include <windows.h>

namespace Chimera {
    static int counter = 0;

    static void show_timer() noexcept;

    void increment_counter() noexcept {
        if(get_tick_count() == 0) {
            counter = 0;
        }
        counter++;
    }

    void reset_counter() noexcept {
        counter = 0;
    }

    bool show_timer_command(int argc, const char **argv) noexcept {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(new_enabled) {
                    add_preframe_event(show_timer, EventPriority::EVENT_PRIORITY_FINAL);
                    add_pretick_event(increment_counter);
                    add_map_load_event(reset_counter);
                }
                else {
                    remove_preframe_event(show_timer);
                    remove_pretick_event(increment_counter);
                    remove_map_load_event(reset_counter);
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));
        return true;
    }

    static void show_timer() noexcept {
        auto current_tick = get_tick_count();
        
        if(counter < current_tick) {
            counter = current_tick;
        }


        char time[64] = "00::00:00.00";

        const int ticks_per_second = 30;
        const int ticks_per_minute =  ticks_per_second * 60;
        const int ticks_per_hour = ticks_per_minute * 60;

        int hours = counter / ticks_per_hour;
        int minutes = (counter - (hours * ticks_per_hour)) / ticks_per_minute;
        float seconds = static_cast<float>(counter - (hours * ticks_per_hour) - (minutes * ticks_per_minute)) / static_cast<float>(ticks_per_second);
        std::snprintf(time, sizeof(time), "%02i:%02i:%05.2f", hours, minutes, seconds);

        static ColorARGB blue = ColorARGB { 0.7, 0.45, 0.72, 1.0 };
        apply_text(time, 170, font_pixel_height(GenericFont::FONT_CONSOLE) + 10, 150, font_pixel_height(GenericFont::FONT_CONSOLE), blue, GenericFont::FONT_CONSOLE, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_BOTTOM_RIGHT);

    }
}
