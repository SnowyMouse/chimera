// SPDX-License-Identifier: GPL-3.0-only

#include "nav_numbers.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"
#include "../event/tick.hpp"

namespace Chimera {
    extern "C" {
        void scoreboard_fade_fix_asm();
    }

    static float *current_value;
    static float fade_time = 0.5F;

    static float scoreboard_up = 0.0F;
    static float scoreboard_down = 0.0F;
    static float scoreboard_current = 0.0F;
    static int scoreboard_direction = 0;

    static void interpolate_fn(float &new_value, float &current_value, float up, float down, float current, int &direction) {
        // If it's the same, do nothing
        float nv = new_value;
        float cv = current_value;
        if(nv == cv) {
            if(cv < 0.5F) {
                direction = -1;
            }
            else {
                direction = 1;
            }
            return;
        }

        // We're going up!
        else if(nv > cv) {
            direction = 1;
            new_value = get_tick_progress() * (up - current) + current;
        }

        // We're going down
        else {
            direction = -1;
            new_value = get_tick_progress() * (down - current) + current;
        }
    }

    extern "C" void interpolate_scoreboard_fade(float *new_value) {
        interpolate_fn(*new_value, *current_value, scoreboard_up, scoreboard_down, scoreboard_current, scoreboard_direction);
    }

    static void normalize_fn(float &up, float &down, float &current, int &direction) {
        switch(direction) {
            case 1:
                current = up;
                break;
            case -1:
                current = down;
                break;
        }

        // Set our range
        float increment = 1.0F / (fade_time * effective_tick_rate());
        down = std::max(current - increment, 0.0F);
        up = std::min(current + increment, 1.0F);
    }

    static void on_tick() {
        normalize_fn(scoreboard_up, scoreboard_down, scoreboard_current, scoreboard_direction);
    }

    void set_up_scoreboard_fade_fix() noexcept {
        auto &chimera = get_chimera();
        std::byte *code_to_use;

        if(chimera.feature_present("client_scoreboard_non_ce")) {
            code_to_use = chimera.get_signature("scoreboard_non_ce_timing_sig").data() + 8;
        }
        else if(chimera.feature_present("client_scoreboard_ce")) {
            code_to_use = chimera.get_signature("scoreboard_ce_timing_sig").data() + 10;
        }
        else {
            return;
        }

        // Get fade time
        fade_time = static_cast<float>(std::max(0.001, chimera.get_ini()->get_value_float("scoreboard.fade_time").value_or(0.5)));

        static Hook hook;
        current_value = *reinterpret_cast<float **>(code_to_use + 3);
        write_jmp_call(code_to_use, hook, reinterpret_cast<const void *>(scoreboard_fade_fix_asm), nullptr, false);
        add_tick_event(on_tick);
    }
}
