// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <optional>
#include "../../command.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../output/draw_text.hpp"
#include "../../../localization/localization.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../event/frame.hpp"
#include "../../../halo_data/keyboard.hpp"
#include "../../../halo_data/main_menu_music.hpp"
#include "../../../halo_data/map.hpp"
#include "../../../halo_data/game_engine.hpp"

namespace Chimera {
    extern "C" void on_error_dialog_asm();
    using clock = std::chrono::steady_clock;
    static clock::time_point next_spam;
    static std::optional<clock::time_point> next_spam_give_up;

    static void force_loading_screen(bool do_it) {
        auto sig = get_chimera().get_signature("do_show_loading_screen_sig");
        const SigByte signature_data[] = { 0xB8, 0x01, 0x00, 0x00, 0x00 };
        if(do_it) {
            write_code_s(sig.data(), signature_data);
        }
        else {
            sig.rollback();
        }
    }

    static void on_frame() {
        apply_text(localize("chimera_spam_to_join_retrying"), -240, 100, 480, 500, ColorARGB(1.0F,1.0F,1.0F,1.0F), GenericFont::FONT_SMALL, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);

        // If excape was invoked, cancel
        if(get_keyboard_keys().escape) {
            remove_preframe_event(on_frame);
            set_force_block_main_menu_music(false);
            force_loading_screen(false);
            get_chimera().execute_command("chimera_load_ui_map");
            return;
        }

        // If it's time to spam, do it
        auto now = clock::now();
        if(now > next_spam && !next_spam_give_up.has_value()) {
            get_chimera().execute_command("chimera_history_connect 1");
            next_spam_give_up = now + std::chrono::milliseconds(500);
        }
        else if(now > next_spam_give_up && next_spam_give_up.has_value()) {
            remove_preframe_event(on_frame);
            set_force_block_main_menu_music(false);
            force_loading_screen(false);
        }
    }

    extern "C" void on_spam_to_join() {
        // Set to 3000 ms because a certain popular server mod thinks it's better to be spamfucked by massive, CPU intensive query packets rather than small connection requests
        next_spam = clock::now() + std::chrono::milliseconds(game_engine() == GameEngine::GAME_ENGINE_DEMO ? 800 : 3000);
        next_spam_give_up = std::nullopt;
        add_preframe_event(on_frame);
        set_force_block_main_menu_music(true);
        force_loading_screen(true);
    }

    bool spam_to_join_command(int argc, const char **argv) {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                active = new_value;
                auto &sig = get_chimera().get_signature("on_error_box_sig");
                if(active) {
                    static Hook hook;
                    write_jmp_call(sig.data(), hook, reinterpret_cast<const void *>(on_error_dialog_asm));
                }
                else {
                    sig.rollback();
                    remove_preframe_event(on_frame);
                }
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
