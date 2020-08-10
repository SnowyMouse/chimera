// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/draw_text.hpp"
#include "../halo_data/tag.hpp"
#include "../output/output.hpp"
#include "../halo_data/resolution.hpp"
#include "../chimera.hpp"

namespace Chimera {
    extern "C" {
        void on_pickup_hud_text_asm() noexcept;
        void on_hold_to_pick_up_hud_text_asm() noexcept;
        void on_weapon_pick_up_hud_text_asm() noexcept;
        void on_hold_to_pick_up_hud_text_button_asm() noexcept;
        void on_names_above_heads_hud_text_asm() noexcept;
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

    extern "C" std::uint32_t on_pickup_hud_text(const wchar_t *string, std::uint32_t xy) noexcept {
        auto &fd = get_current_font_data();

        auto x = xy >> 16;
        auto y = (xy & 0xFFFF);

        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT);
        return y + hud_line_size();
    }

    extern "C" void on_hold_hud_text(const wchar_t *string, std::uint32_t *xy, std::uint32_t element) noexcept {
        auto &fd = get_current_font_data();

        auto large_font_tag_id = get_generic_font(GenericFont::FONT_LARGE);
        auto x = (*xy >> 16) - text_pixel_length(string, large_font_tag_id);
        auto y = (*xy & 0xFFFF);

        // If we're the first element, we should offset the Y coordinate based on the differences of our font vs. the tag font
        if(element == 0) {
            y = y - font_pixel_height(large_font_tag_id) + font_pixel_height(GenericFont::FONT_LARGE);
        }

        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT);

        *xy = static_cast<std::uint16_t>(y);
        *xy |= (x + (text_pixel_length(string, GenericFont::FONT_LARGE))) << 16;
    }

    extern "C" void on_weapon_pick_up_hud_text(const wchar_t *string, std::uint32_t *xy) noexcept {
        auto &fd = get_current_font_data();

        auto large_font_tag_id = get_generic_font(GenericFont::FONT_LARGE);
        auto x = (*xy >> 16);
        // Again, offset the Y coordinate based on the differences of our font vs. the tag font for consistency
        auto y = (*xy & 0xFFFF) - font_pixel_height(large_font_tag_id) + font_pixel_height(GenericFont::FONT_LARGE);
        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT);

        *xy = static_cast<std::uint16_t>(y);
        *xy |= x << 16;
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
        auto width = text_pixel_length(string, GenericFont::FONT_SMALL);
        apply_text(std::wstring(string), x_middle - width / 2, y, width, 1024, fd.color, GenericFont::FONT_SMALL, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT);
    }

    static bool enabled = false;

    bool hud_text_mod_initialized() noexcept {
        return enabled;
    }

    void initialize_hud_text() noexcept {
        enabled = true;

        auto &chimera = get_chimera();

        static SigByte nop_fn[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

        static Hook picked_up_ammo;
        auto *picked_up_ammo_draw_text_call = chimera.get_signature("picked_up_ammo_draw_text_call_sig").data() + 11;
        write_code_s(picked_up_ammo_draw_text_call, nop_fn);
        write_jmp_call(picked_up_ammo_draw_text_call, picked_up_ammo, reinterpret_cast<const void *>(on_pickup_hud_text_asm), nullptr, false);

        static Hook hold_text;
        auto *hold_to_pick_up_text_call_sig = chimera.get_signature("hold_to_pick_up_text_call_sig").data() + 14;
        write_code_s(hold_to_pick_up_text_call_sig, nop_fn);
        write_jmp_call(hold_to_pick_up_text_call_sig, hold_text, reinterpret_cast<const void *>(on_hold_to_pick_up_hud_text_asm), nullptr, false);

        static Hook button_text;
        auto *hold_button_text_call_sig = chimera.get_signature("hold_button_text_call_sig").data() + 11;
        write_code_s(hold_button_text_call_sig, nop_fn);
        write_jmp_call(hold_button_text_call_sig, button_text, reinterpret_cast<const void *>(on_hold_to_pick_up_hud_text_button_asm), nullptr, false);

        static Hook picked_up_weapon;
        auto *picked_up_a_weapon_text_call_sig = chimera.get_signature("picked_up_a_weapon_text_call_sig").data() + 7;
        write_code_s(picked_up_a_weapon_text_call_sig, nop_fn);
        write_jmp_call(picked_up_a_weapon_text_call_sig, picked_up_weapon, reinterpret_cast<const void *>(on_weapon_pick_up_hud_text_asm), nullptr, false);

        static Hook widescreen_text_f3_name;
        auto *widescreen_text_f3_name_sig = chimera.get_signature("widescreen_text_f3_name_sig").data();
        write_code_s(widescreen_text_f3_name_sig, nop_fn);
        write_jmp_call(widescreen_text_f3_name_sig, widescreen_text_f3_name, reinterpret_cast<const void *>(on_names_above_heads_hud_text_asm), nullptr, false);
    }
}
