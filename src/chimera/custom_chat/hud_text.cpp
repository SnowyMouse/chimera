// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/draw_text.hpp"
#include "../halo_data/tag.hpp"
#include "../output/output.hpp"
#include "../chimera.hpp"

namespace Chimera {
    extern "C" {
        void on_pickup_hud_text_asm() noexcept;
        void on_hold_to_pick_up_hud_text_asm() noexcept;
    }

    static const std::byte *hud_globals_data() noexcept {
        auto *tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *tag_data = tag->data;
        auto *interface_bitmaps = *reinterpret_cast<const std::byte **>(tag_data + 0x140 + 4);
        auto &hud_globals = *reinterpret_cast<const TagID *>(interface_bitmaps + 0x60 + 0xC);
        auto *hud_globals_tag = get_tag(hud_globals);
        return hud_globals_tag->data;
    }

    //const ColorARGB &get_hud_text_color() noexcept {
    //    return *reinterpret_cast<const ColorARGB *>(hud_globals_data() + 0x80);
    //}

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

    extern "C" void on_hold_hud_text(const wchar_t *string, std::uint32_t *xy) noexcept {
        //*xy = 0;
        auto &fd = get_current_font_data();
        auto x = (*xy >> 16) - text_pixel_length(string, get_generic_font(GenericFont::FONT_LARGE));
        auto y = (*xy & 0xFFFF);
        apply_text(std::wstring(string), x, y, 1024, 1024, fd.color, GenericFont::FONT_LARGE, fd.alignment, TextAnchor::ANCHOR_TOP_LEFT);

        *xy &= 0x0000FFFF;
        *xy |= (x + (text_pixel_length(string, GenericFont::FONT_LARGE))) << 16;
    }

    void initialize_hud_text() noexcept {
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


    }
}
