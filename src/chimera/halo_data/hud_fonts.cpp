// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../output/draw_text.hpp"
#include "../halo_data/tag.hpp"
#include "../output/output.hpp"
#include "../halo_data/resolution.hpp"
#include "../fix/widescreen_fix.hpp"
#include "../event/frame.hpp"
#include "../chimera.hpp"

#include "hud_fonts.hpp"

namespace Chimera {
    static GenericFont scoreboard_font;

    extern "C" void get_scoreboard_font_esi_asm() noexcept;
    extern "C" void get_scoreboard_font_edx_asm() noexcept;
    extern "C" std::uint32_t get_scoreboard_font() {
        return get_generic_font(scoreboard_font).whole_id;
    }

    extern "C" std::uint32_t get_scoreboard_font_generic() noexcept {
        return scoreboard_font;
    }

    static GenericFont name_font;

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

    extern "C" {
        void on_pickup_hud_text_asm() noexcept;
        void on_hud_text_esi_asm() noexcept;
        void on_weapon_pick_up_hud_text_asm() noexcept;
        void on_names_above_heads_hud_text_asm() noexcept;
        void on_stare_hud_text_asm() noexcept;
        void hud_text_fmul_with_0_asm() noexcept;
        void on_menu_hud_text_double_scaled_asm() noexcept;
        void on_menu_hud_text_double_scaled_large_asm() noexcept;
        void on_menu_hud_text_asm() noexcept;
        void on_menu_hud_text_large_asm() noexcept;
        void on_menu_hud_text_scoreboard_asm() noexcept;
        void on_menu_hud_text_unscaled_asm() noexcept;
        void on_menu_hud_text_unscaled_large_asm() noexcept;
        float hud_text_new_line_spacing = 0.0F;
    }

    static const std::byte *hud_globals_data() noexcept {
        auto *tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *tag_data = tag->data;
        auto *interface_bitmaps = *reinterpret_cast<const std::byte **>(tag_data + 0x140 + 4);
        auto &hud_globals = *reinterpret_cast<const TagID *>(interface_bitmaps + 0x60 + 0xC);
        auto *hud_globals_tag = get_tag(hud_globals);
        return hud_globals_tag->data;
    }

    static std::uint32_t hud_line_size() noexcept {
        return *reinterpret_cast<const float *>(hud_globals_data() + 0x90) * font_pixel_height(GenericFont::FONT_LARGE);
    }

    static void on_frame() noexcept {
        hud_text_new_line_spacing = hud_line_size();
    }

    extern "C" void on_pickup_hud_text(const wchar_t *string, std::uint32_t xy) noexcept {
        auto &fd = get_current_font_data();

        auto x = xy >> 16;
        auto y = (xy & 0xFFFF);

        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);
    }

    extern "C" void on_hold_hud_text(const wchar_t *string, std::uint32_t *xy_to, std::uint32_t xy) noexcept {
        auto &fd = get_current_font_data();

        auto x = xy >> 16;
        auto y = xy & 0xFFFF;
        auto font = GenericFont::FONT_LARGE;

        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, font, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);

        // Do we have a newline? If so, reset xy_to. Otherwise, continue it
        bool new_line_present = false;
        std::size_t next_line_length = 0;
        for(auto *c = string; *c; c++) {
            std::size_t l = 0;
            if(*c == '\n') {
                l = 1;
            }
            else if(*c == '|' && c[1] == 'n') {
                l = 2;
            }
            if(l) {
                new_line_present = true;
                next_line_length = text_pixel_length(c + l, font);
            }
        }

        // If we do have a newline, we need to put the cursor at the next line after the text we just drew
        if(new_line_present) {
            *xy_to = static_cast<std::uint16_t>(y + font_pixel_height(font));
            *xy_to |= ((xy >> 16) + next_line_length) << 16;
        }
        else {
            *xy_to = static_cast<std::uint16_t>(y);
            *xy_to |= (x + (fd.xy_offset >> 16) + (text_pixel_length(string, font))) << 16;
        }
    }

    extern "C" void on_weapon_pick_up_hud_text(const wchar_t *string, std::uint32_t xy) noexcept {
        auto &fd = get_current_font_data();

        auto x = (xy >> 16);
        auto y = (xy & 0xFFFF);
        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);
    }

    extern "C" void on_names_above_heads_hud_text(const wchar_t *string, std::uint32_t *xy, std::uint32_t stare) noexcept {
        auto &fd = get_current_font_data();

        auto res = get_resolution();
        auto x0 = xy[0] >> 16;
        auto x1 = xy[1] >> 16;
        auto avg = (x0 + x1) / 2;
        double scale = widescreen_fix_enabled() ? ((static_cast<float>(res.width) / static_cast<float>(res.height) * 480) / 640.0) : 1.0;
        auto x_middle = avg * scale;

        auto y = *xy & 0xFFFF;
        auto font_to_use = (!stare) ? GenericFont::FONT_SMALLER : name_font;
        auto width = text_pixel_length(string, font_to_use);
        apply_text(std::wstring(string), x_middle - width / 2, y, width, 1024, fd.color, font_to_use, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);
    }

    extern "C" void on_menu_hud_text(const wchar_t *string, std::uint32_t *xy, std::uint32_t scale, std::uint32_t force) noexcept {
        auto &fd = get_current_font_data();

        auto res = get_resolution();
        int add = widescreen_fix_enabled() ? (scale * (static_cast<std::int16_t>((static_cast<double>(res.width) / res.height) * 480.000f - 640.000f) / 2.0f)) : 0;

        std::variant<TagID, GenericFont> font;

        // Determine the font tag based on the tag path
        if(force == 0xFFFFFFFF) {
            auto *tag = get_tag(fd.font);
            if(tag == nullptr) {
                std::printf("You just tried to call bullshit. Get fucked.\n");
                return;
            }
            auto *tag_path = tag->path;
            if(std::strcmp(tag_path, "ui\\large_ui") == 0) {
                font = GenericFont::FONT_LARGE;
            }
            else if(std::strcmp(tag_path, "ui\\small_ui") == 0) {
                font = GenericFont::FONT_SMALL;
            }
            else if(std::strcmp(tag_path, "ui\\gamespy") == 0) {
                font = GenericFont::FONT_SMALLER;
            }
            else if(std::strcmp(tag_path, "ui\\ticker") == 0) {
                font = GenericFont::FONT_TICKER;
            }
            else {
                font = fd.font; // give up - use a crappy bitmap font instead
            }
        }

        // Some things should just be the same font regardless of map
        else {
            font = static_cast<GenericFont>(force);
        }

        auto x1 = (*xy >> 16) + add;
        auto y1 = (*xy & 0xFFFF);

        auto x2 = (xy[1] >> 16) + add;
        auto y2 = (xy[1] & 0xFFFF);

        apply_text(std::wstring(string), x1, y1, x2 - x1, y2 - y1, fd.color, font, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);
    }

    static bool enabled = false;

    bool hud_text_mod_initialized() noexcept {
        return enabled;
    }

    void initialize_hud_text() noexcept {
        enabled = true;
        set_widescreen_fix(WidescreenFixSetting::WIDESCREEN_ON);

        auto &chimera = get_chimera();

        add_preframe_event(on_frame);

        static SigByte nop_fn[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

        // Picked up %i rounds
        static Hook picked_up_ammo;
        auto *picked_up_ammo_draw_text_call = chimera.get_signature("picked_up_ammo_draw_text_call_sig").data() + 11;
        write_code_s(picked_up_ammo_draw_text_call, nop_fn);
        write_jmp_call(picked_up_ammo_draw_text_call, picked_up_ammo, reinterpret_cast<const void *>(on_pickup_hud_text_asm), nullptr, false);

        // "Hold" "to pick up"
        static Hook hold_text;
        auto *hold_to_pick_up_text_call_sig = chimera.get_signature("hold_to_pick_up_text_call_sig").data() + 14;
        write_code_s(hold_to_pick_up_text_call_sig, nop_fn);
        write_jmp_call(hold_to_pick_up_text_call_sig, hold_text, reinterpret_cast<const void *>(on_hud_text_esi_asm), nullptr, false);

        // This is for the button part
        static Hook button_text;
        auto *hold_button_text_call_sig = chimera.get_signature("hold_button_text_call_sig").data() + 11;
        write_code_s(hold_button_text_call_sig, nop_fn);
        write_jmp_call(hold_button_text_call_sig, button_text, reinterpret_cast<const void *>(on_hud_text_esi_asm), nullptr, false);

        // Picked up weapon
        static Hook picked_up_weapon;
        auto *picked_up_a_weapon_text_call_sig = chimera.get_signature("picked_up_a_weapon_text_call_sig").data() + 7;
        write_code_s(picked_up_a_weapon_text_call_sig, nop_fn);
        write_jmp_call(picked_up_a_weapon_text_call_sig, picked_up_weapon, reinterpret_cast<const void *>(on_weapon_pick_up_hud_text_asm), nullptr, false);

        // F3 stuff
        if(chimera.feature_present("core_custom_edition")) {
            static Hook widescreen_text_f3_name;
            auto *widescreen_text_f3_name_sig = chimera.get_signature("widescreen_text_f3_name_sig").data();
            write_code_s(widescreen_text_f3_name_sig, nop_fn);
            write_jmp_call(widescreen_text_f3_name_sig, widescreen_text_f3_name, reinterpret_cast<const void *>(on_names_above_heads_hud_text_asm), nullptr, false);
        }

        // Stare text
        static Hook stare_name;
        auto *widescreen_text_stare_name_sig = chimera.get_signature("widescreen_text_stare_name_sig").data();
        write_code_s(widescreen_text_stare_name_sig, nop_fn);
        write_jmp_call(widescreen_text_stare_name_sig, stare_name, reinterpret_cast<const void *>(on_stare_hud_text_asm), nullptr, false);

        // Fix multiplayer text
        static Hook hold_f1;
        auto *multiplayer_spawn_timer_hold_f1_for_score_text_call_sig = chimera.get_signature("multiplayer_spawn_timer_hold_f1_for_score_text_call_sig").data() + 14;
        write_code_s(multiplayer_spawn_timer_hold_f1_for_score_text_call_sig, nop_fn);
        write_jmp_call(multiplayer_spawn_timer_hold_f1_for_score_text_call_sig, hold_f1, reinterpret_cast<const void *>(on_hud_text_esi_asm), nullptr, false);

        // Menu text
        static Hook widescreen_menu_text;
        auto *widescreen_menu_text_sig = chimera.get_signature("widescreen_menu_text_sig").data() + 9;
        write_code_s(widescreen_menu_text_sig, nop_fn);
        write_jmp_call(widescreen_menu_text_sig, widescreen_menu_text, reinterpret_cast<const void *>(on_menu_hud_text_asm), nullptr, false);

        // Text shown when connecting (hardcode to large)
        static Hook connection_text;
        auto *connecting_text_call_sig = chimera.get_signature("connecting_text_call_sig").data() + 9;
        write_code_s(connecting_text_call_sig, nop_fn);
        write_jmp_call(connecting_text_call_sig, connection_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_large_asm), nullptr, false);

        static Hook esrb_text;
        auto *esrb_text_call_sig = chimera.get_signature("esrb_text_call_sig").data() + 9;
        write_code_s(esrb_text_call_sig, nop_fn);
        write_jmp_call(esrb_text_call_sig, esrb_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_large_asm), nullptr, false);

        static Hook hold_to_cancel_text;
        auto *hold_to_cancel_connect_sig = chimera.get_signature("hold_to_cancel_connect_text_call_sig").data() + 9;
        write_code_s(hold_to_cancel_connect_sig, nop_fn);
        write_jmp_call(hold_to_cancel_connect_sig, hold_to_cancel_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_large_asm), nullptr, false);

        static Hook connecting_to_server_text;
        auto *connecting_to_server_text_call_sig = chimera.get_signature("connecting_to_server_text_call_sig").data() + 9;
        write_code_s(connecting_to_server_text_call_sig, nop_fn);
        write_jmp_call(connecting_to_server_text_call_sig, connecting_to_server_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_large_asm), nullptr, false);

        static Hook hosting_server_text;
        auto *hosting_server_text_call_sig = chimera.get_signature("hosting_server_text_call_sig").data() + 9;
        write_code_s(hosting_server_text_call_sig, nop_fn);
        write_jmp_call(hosting_server_text_call_sig, hosting_server_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_large_asm), nullptr, false);

        static Hook hosting_loading_map_text;
        auto *hosting_loading_map_text_call_sig = chimera.get_signature("hosting_loading_map_text_call_sig").data() + 13;
        write_code_s(hosting_loading_map_text_call_sig, nop_fn);
        write_jmp_call(hosting_loading_map_text_call_sig, hosting_loading_map_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_large_asm), nullptr, false);

        static Hook connection_established_text;
        auto *connection_established_text_call_sig = chimera.get_signature("connection_established_text_call_sig").data() + 13;
        write_code_s(connection_established_text_call_sig, nop_fn);
        write_jmp_call(connection_established_text_call_sig, connection_established_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_large_asm), nullptr, false);

        // Postgame carnage report text (hardcode to large)
        static Hook pgcr_text;
        auto *widescreen_text_pgcr_sig = chimera.get_signature("widescreen_text_pgcr_sig").data();
        write_code_s(widescreen_text_pgcr_sig, nop_fn);
        write_jmp_call(widescreen_text_pgcr_sig, pgcr_text, reinterpret_cast<const void *>(on_menu_hud_text_large_asm), nullptr, false);

        // F1 stuff (use scoreboard or small)
        static Hook f1_text;
        bool demo = chimera.feature_present("client_demo");
        auto *widescreen_text_f1_sig = chimera.get_signature(demo ? "widescreen_text_f1_demo_sig" : "widescreen_text_f1_sig").data();
        write_code_s(widescreen_text_f1_sig, nop_fn);
        write_jmp_call(widescreen_text_f1_sig, f1_text, reinterpret_cast<const void *>(on_menu_hud_text_scoreboard_asm), nullptr, false);

        // Server name & IP stuff (hardcode to large)
        static Hook f1_server_name;
        std::byte *server_name_text_call_sig;
        if(chimera.feature_present("client_custom_edition")) {
            server_name_text_call_sig = chimera.get_signature("server_name_text_call_custom_edition_sig").data() + 10;
        }
        else if(demo) {
            server_name_text_call_sig = chimera.get_signature("server_name_text_call_demo_sig").data() + 6;
        }
        else {
            server_name_text_call_sig = chimera.get_signature("server_name_text_call_retail_sig").data() + 6;
        }
        write_code_s(server_name_text_call_sig, nop_fn);
        write_jmp_call(server_name_text_call_sig, f1_server_name, reinterpret_cast<const void *>(on_menu_hud_text_double_scaled_large_asm), nullptr, false);

        static Hook f1_ip;
        auto *server_ip_text_call_sig = chimera.get_signature("server_ip_text_call_sig").data() + 10;
        write_code_s(server_ip_text_call_sig, nop_fn);
        write_jmp_call(server_ip_text_call_sig, f1_ip, reinterpret_cast<const void *>(on_menu_hud_text_double_scaled_large_asm), nullptr, false);

        // Prompt text
        static Hook main_menu_prompt_text;
        auto *main_menu_prompt_text_sig = chimera.get_signature("main_menu_prompt_text_sig").data() + 28;
        write_code_s(main_menu_prompt_text_sig, nop_fn);
        write_jmp_call(main_menu_prompt_text_sig, main_menu_prompt_text, reinterpret_cast<const void *>(on_menu_hud_text_asm), nullptr, false);

        // Cutscene text
        static Hook cutscene_text;
        auto *widescreen_text_cutscene_sig = chimera.get_signature("widescreen_text_cutscene_sig").data() + 8;
        write_code_s(widescreen_text_cutscene_sig, nop_fn);
        write_jmp_call(widescreen_text_cutscene_sig, cutscene_text, reinterpret_cast<const void *>(on_menu_hud_text_double_scaled_asm), nullptr, false);

        // Enter/Cancel buttons
        static Hook main_menu_text_input;
        auto *main_menu_text_input_sig = chimera.get_signature("main_menu_text_input_sig").data() + 24;
        write_code_s(main_menu_text_input_sig, nop_fn);
        write_jmp_call(main_menu_text_input_sig, main_menu_text_input, reinterpret_cast<const void *>(on_menu_hud_text_asm), nullptr, false);

        // Menu text
        static Hook main_menu_text_input_text;
        auto *widescreen_input_text_sig = get_chimera().get_signature("widescreen_input_text_sig").data();
        write_code_s(widescreen_input_text_sig, nop_fn);
        write_jmp_call(widescreen_input_text_sig, main_menu_text_input_text, reinterpret_cast<const void *>(on_menu_hud_text_asm), nullptr, false);

        static Hook widescreen_menu_2_text;
        auto *widescreen_menu_text_2_sig = chimera.get_signature("widescreen_menu_text_2_sig").data();
        write_code_s(widescreen_menu_text_2_sig, nop_fn);
        write_jmp_call(widescreen_menu_text_2_sig, widescreen_menu_2_text, reinterpret_cast<const void *>(on_menu_hud_text_asm), nullptr, false);

        // Make the line spacing use our font instead of the map's font
        static Hook line_spacing_1, line_spacing_2;
        static SigByte nop_flt[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        auto *line_spacing_draw_text_1_sig = chimera.get_signature("line_spacing_draw_text_1_sig").data();
        auto *line_spacing_draw_text_2_sig = chimera.get_signature("line_spacing_draw_text_2_sig").data();
        write_code_s(line_spacing_draw_text_1_sig, nop_flt);
        write_code_s(line_spacing_draw_text_2_sig, nop_flt);
        write_jmp_call(line_spacing_draw_text_1_sig, line_spacing_1, reinterpret_cast<const void *>(hud_text_fmul_with_0_asm), nullptr, false);
        write_jmp_call(line_spacing_draw_text_2_sig, line_spacing_2, reinterpret_cast<const void *>(hud_text_fmul_with_0_asm), nullptr, false);
    }
}
