// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/draw_text.hpp"
#include "../halo_data/tag.hpp"
#include "../output/output.hpp"
#include "../halo_data/resolution.hpp"
#include "../event/frame.hpp"
#include "../chimera.hpp"

namespace Chimera {
    extern "C" {
        void on_pickup_hud_text_asm() noexcept;
        void on_hud_text_esi_asm() noexcept;
        void on_weapon_pick_up_hud_text_asm() noexcept;
        void on_names_above_heads_hud_text_asm() noexcept;
        void hud_text_fmul_with_0_asm() noexcept;
        void on_menu_hud_text_asm() noexcept;
        void on_menu_hud_text_unscaled_asm() noexcept;
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

    extern "C" void on_hold_hud_text(const wchar_t *string, std::uint32_t *xy) noexcept {
        auto &fd = get_current_font_data();

        auto large_font_tag_id = get_generic_font(GenericFont::FONT_LARGE);
        auto x = (*xy >> 16) - text_pixel_length(string, large_font_tag_id);
        auto y = (*xy & 0xFFFF);

        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);

        *xy = static_cast<std::uint16_t>(y);
        *xy |= (x + (text_pixel_length(string, GenericFont::FONT_LARGE))) << 16;
    }

    extern "C" void on_weapon_pick_up_hud_text(const wchar_t *string, std::uint32_t xy) noexcept {
        auto &fd = get_current_font_data();

        auto x = (xy >> 16);
        auto y = (xy & 0xFFFF);
        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);
    }

    extern "C" void on_names_above_heads_hud_text(const wchar_t *string, std::uint32_t *xy) noexcept {
        auto &fd = get_current_font_data();

        auto res = get_resolution();
        auto x0 = xy[0] >> 16;
        auto x1 = xy[1] >> 16;
        auto avg = (x0 + x1) / 2;
        float scale = (static_cast<float>(res.width) / static_cast<float>(res.height) * 480) / 640.0;
        auto x_middle = avg * scale;

        auto y = *xy & 0xFFFF;
        auto font_to_use = fd.font == get_generic_font(GenericFont::FONT_SMALLER) ? GenericFont::FONT_SMALLER : GenericFont::FONT_SMALL;
        auto width = text_pixel_length(string, font_to_use);
        apply_text(std::wstring(string), x_middle - width / 2, y, width, 1024, fd.color, font_to_use, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT, true);
    }

    extern "C" void on_menu_hud_text(const wchar_t *string, std::uint32_t *xy, std::uint32_t scale) noexcept {
        auto &fd = get_current_font_data();

        auto res = get_resolution();
        int add = scale ? (static_cast<std::int16_t>((static_cast<double>(res.width) / res.height) * 480.000f - 640.000f) / 2.0f) : 0;

        GenericFont font;
        if(fd.font == get_generic_font(GenericFont::FONT_LARGE)) {
            font = GenericFont::FONT_LARGE;
        }
        else if(fd.font == get_generic_font(GenericFont::FONT_SMALL)) {
            font = GenericFont::FONT_SMALL;
        }
        else if(fd.font == get_generic_font(GenericFont::FONT_SMALLER)) {
            font = GenericFont::FONT_SMALLER;
        }
        else if(fd.font == get_generic_font(GenericFont::FONT_TICKER)) {
            font = GenericFont::FONT_TICKER;
        }
        else {
            font = GenericFont::FONT_SMALL;
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

        static Hook stare_name;
        auto *widescreen_text_stare_name_sig = chimera.get_signature("widescreen_text_stare_name_sig").data();
        write_code_s(widescreen_text_stare_name_sig, nop_fn);
        write_jmp_call(widescreen_text_stare_name_sig, stare_name, reinterpret_cast<const void *>(on_names_above_heads_hud_text_asm), nullptr, false);

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

        static Hook connection_text;
        auto *connecting_text_call_sig = chimera.get_signature("connecting_text_call_sig").data() + 9;
        write_code_s(connecting_text_call_sig, nop_fn);
        write_jmp_call(connecting_text_call_sig, connection_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_asm), nullptr, false);

        static Hook esrb_text;
        auto *esrb_text_call_sig = chimera.get_signature("esrb_text_call_sig").data() + 9;
        write_code_s(esrb_text_call_sig, nop_fn);
        write_jmp_call(esrb_text_call_sig, esrb_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_asm), nullptr, false);

        static Hook hold_to_cancel_text;
        auto *hold_to_cancel_connect_sig = chimera.get_signature("hold_to_cancel_connect_text_call_sig").data() + 9;
        write_code_s(hold_to_cancel_connect_sig, nop_fn);
        write_jmp_call(hold_to_cancel_connect_sig, hold_to_cancel_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_asm), nullptr, false);

        static Hook connecting_to_server_text;
        auto *connecting_to_server_text_call_sig = chimera.get_signature("connecting_to_server_text_call_sig").data() + 9;
        write_code_s(connecting_to_server_text_call_sig, nop_fn);
        write_jmp_call(connecting_to_server_text_call_sig, connecting_to_server_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_asm), nullptr, false);

        static Hook main_menu_prompt_text;
        auto *main_menu_prompt_text_sig = chimera.get_signature("main_menu_prompt_text_sig").data() + 28;
        write_code_s(main_menu_prompt_text_sig, nop_fn);
        write_jmp_call(main_menu_prompt_text_sig, main_menu_prompt_text, reinterpret_cast<const void *>(on_menu_hud_text_asm), nullptr, false);

        static Hook hosting_server_text;
        auto *hosting_server_text_call_sig = chimera.get_signature("hosting_server_text_call_sig").data() + 9;
        write_code_s(hosting_server_text_call_sig, nop_fn);
        write_jmp_call(hosting_server_text_call_sig, hosting_server_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_asm), nullptr, false);

        static Hook hosting_loading_map_text;
        auto *hosting_loading_map_text_call_sig = chimera.get_signature("hosting_loading_map_text_call_sig").data() + 13;
        write_code_s(hosting_loading_map_text_call_sig, nop_fn);
        write_jmp_call(hosting_loading_map_text_call_sig, hosting_loading_map_text, reinterpret_cast<const void *>(on_menu_hud_text_unscaled_asm), nullptr, false);

        // Enter/Cancel buttons
        static Hook main_menu_text_input;
        auto *main_menu_text_input_sig = chimera.get_signature("main_menu_text_input_sig").data() + 24;
        write_code_s(main_menu_text_input_sig, nop_fn);
        write_jmp_call(main_menu_text_input_sig, main_menu_text_input, reinterpret_cast<const void *>(on_menu_hud_text_asm), nullptr, false);

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
