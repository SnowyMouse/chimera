#include <optional>
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../output/draw_text.hpp"

#include "hud_fonts.hpp"

namespace Chimera {
    GenericFont scoreboard_font;

    extern "C" void get_scoreboard_font_esi_asm() noexcept;
    extern "C" void get_scoreboard_font_edx_asm() noexcept;
    extern "C" std::uint32_t get_scoreboard_font() {
        return get_generic_font(scoreboard_font).whole_id;
    }

    GenericFont name_font;

    //extern "C" void get_scoreboard_font_esi_asm() noexcept;
    extern "C" void get_name_font_eax_asm() noexcept;
    extern "C" std::uint32_t get_name_font() {
        return get_generic_font(scoreboard_font).whole_id;
    }

    void set_up_scoreboard_font() noexcept {
        auto &chimera = get_chimera();
        auto *ini = chimera.get_ini();

        // Get the generic font set (if it is set)
        auto *font = ini->get_value("scoreboard.font");
        if(!font) {
            return;
        }

        scoreboard_font = generic_font_from_string(font);
        static Hook hook;

        // Non-trial
        if(chimera.feature_present("client_score_screen")) {
            auto &ss_elements_sig_b = chimera.get_signature("ss_elements_sig_b");
            write_jmp_call(ss_elements_sig_b.data(), hook, reinterpret_cast<const void *>(get_scoreboard_font_esi_asm), nullptr, false);
        }

        // Trial
        else if(chimera.feature_present("client_score_screen_font_demo")) {
            auto &ss_elements_font_demo_sig = chimera.get_signature("ss_elements_font_demo_sig");
            write_jmp_call(ss_elements_font_demo_sig.data(), hook, nullptr, reinterpret_cast<const void *>(get_scoreboard_font_edx_asm), false);
        }
    }

    void set_up_name_font() noexcept {
        auto &chimera = get_chimera();
        auto *ini = chimera.get_ini();

        // Get the generic font set (if it is set)
        auto *font = ini->get_value("name.font");
        if(!font) {
            return;
        }

        name_font = generic_font_from_string(font);
        static Hook hook;

        if(chimera.feature_present("client_name_font")) {
            auto &name_font_demo_sig = chimera.get_signature("name_font_sig");
            write_jmp_call(name_font_demo_sig.data(), hook, nullptr, reinterpret_cast<const void *>(get_name_font_eax_asm), false);
        }
    }
}
