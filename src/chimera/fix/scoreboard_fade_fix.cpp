#include "nav_numbers.hpp"
#include "../chimera.hpp"
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
    static float fade_amount_per_tick = 2.0F / 30.0F;

    extern "C" void interpolate_scoreboard_fade(float *new_value) {
        float value_diff = fade_amount_per_tick * (*new_value > value_current_tick ? 1.0F : -1.0F);
        float value_next_tick = value_current_tick + value_diff;
        float v = value_current_tick + (value_next_tick - value_current_tick) * get_tick_progress();
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

        static Hook hook;
        current_value = *reinterpret_cast<float **>(code_to_use + 3);
        write_jmp_call(code_to_use, hook, reinterpret_cast<const void *>(scoreboard_fade_fix_asm), nullptr, false);
        add_tick_event(on_tick);
    }
}
