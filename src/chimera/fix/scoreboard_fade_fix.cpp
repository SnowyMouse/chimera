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
    static float value_current_tick = 0.0F;
    static float fade_time = 0.5F;

    extern "C" void interpolate_scoreboard_fade(float *new_value) {
        float fade_amount_per_tick = 1.0F / (fade_time * effective_tick_rate());

        float sign;

        // If it's the same, do nothing
        float nv = *new_value;
        float cv = *current_value;
        if(nv == cv) {
            return;
        }

        // We're going up!
        else if(nv > cv) {
            sign = 1.0F;
        }

        // We're going down
        else {
            sign = -1.0F;
        }

        // Now let's see how far we take it
        float value_diff = fade_amount_per_tick * sign;
        float value_next_tick = value_current_tick + value_diff;
        float v = value_current_tick + (value_next_tick - value_current_tick) * get_tick_progress();

        // Okay!
        if(v > 1.0F) {
            v = 1.0F;
        }
        else if(v < 0.0F) {
            v = 0.0F;
        }

        *new_value = v;
    }

    static void on_tick() {
        value_current_tick = *current_value;
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
        auto *fade_time_ini = chimera.get_ini()->get_value("scoreboard.fade_time");
        if(fade_time_ini) {
            try {
                float new_fade_time = std::strtof(fade_time_ini, nullptr);
                fade_time = std::max(0.001F, new_fade_time);
            }
            catch(std::exception &) {
                // it's invalid; do nothing
            }
        }

        static Hook hook;
        current_value = *reinterpret_cast<float **>(code_to_use + 3);
        write_jmp_call(code_to_use, hook, reinterpret_cast<const void *>(scoreboard_fade_fix_asm), nullptr, false);
        add_tick_event(on_tick);
    }
}
