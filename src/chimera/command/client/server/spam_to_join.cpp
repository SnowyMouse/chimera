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
#include "../../../bookmark/bookmark.hpp"

namespace Chimera {
    extern "C" void on_error_dialog_asm();
    using clock = std::chrono::steady_clock;
    static clock::time_point next_spam;
    static std::optional<clock::time_point> next_spam_give_up;

    static bool checked_query_packet = false;
    static std::optional<QueryPacketDone> latest_query_packet;

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

    // Check if we're connecting to a SAPP server. SAPP throttles connections because, for whatever reason, it's better to send a massive fuckton of text that is CPU-intensive to generate in a full query packet than simply say "I'm full" to a connection request
    static bool requery_sapp_server() {
        if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
            latest_query_packet = std::nullopt;
            return false;
        }
        latest_query_packet = query_server(get_latest_connection());
        if(latest_query_packet->error || latest_query_packet->get_data_for_key("sappflags")) {
            latest_query_packet = std::nullopt;
            return false;
        }
        return true;
    }

    static void on_frame() {
        apply_text(localize("chimera_spam_to_join_retrying"), -240, 100, 480, 500, ColorARGB(1.0F,1.0F,1.0F,1.0F), GenericFont::FONT_SMALL, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);

        // If excape was invoked, cancel
        if(get_keyboard_keys().escape) {
            remove_preframe_event(on_frame);
            set_force_block_main_menu_music(false);
            force_loading_screen(false);
            get_chimera().execute_command("chimera_load_ui_map");
            checked_query_packet = false;
            return;
        }

        // If it's time to spam, do it
        auto now = clock::now();
        if(now > next_spam && !next_spam_give_up.has_value()) {
            bool retry;
            if(latest_query_packet.has_value() && requery_sapp_server()) {
                // If it's a SAPP server, retry if max players is greater than num players
                try {
                    auto num = std::stoi(latest_query_packet->get_data_for_key("numplayers"));
                    auto max = std::stoi(latest_query_packet->get_data_for_key("maxplayers"));
                    retry = max > num;
                }
                catch(std::exception &) {
                    retry = true;
                }
            }
            else {
                retry = true;
            }

            // Retry if needed
            if(retry) {
                get_chimera().execute_command("chimera_history_connect 1");
                next_spam_give_up = now + std::chrono::milliseconds(500);
            }
            else {
                next_spam = clock::now() + std::chrono::milliseconds(800);
            }
        }
        else if(now > next_spam_give_up && next_spam_give_up.has_value()) {
            remove_preframe_event(on_frame);
            set_force_block_main_menu_music(false);
            force_loading_screen(false);
            checked_query_packet = false;
        }
    }

    extern "C" void on_spam_to_join() {
        if(!checked_query_packet) {
            checked_query_packet = true;
            requery_sapp_server();
        }

        next_spam = clock::now() + std::chrono::milliseconds(800);
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
