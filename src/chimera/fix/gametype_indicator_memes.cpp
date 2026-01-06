// SPDX-License-Identifier: GPL-3.0-only

#include "gametype_indicator_memes.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../event/frame.hpp"
#include "../event/map_load.hpp"
#include "../math_trig/math_trig.hpp"


namespace Chimera {
    extern "C" {
        void hud_render_gametype_draw_this_frame_asm() noexcept;
        const void *original_gametype_draw_instr = nullptr;
        std::byte *skip_gametype_draw_ptr = nullptr;

        void hud_render_get_tag_for_ctf() noexcept;
        void hud_render_get_tag_for_slayer() noexcept;
        void hud_render_get_tag_for_oddball() noexcept;
        void hud_render_get_tag_for_king() noexcept;
        void hud_render_get_tag_for_race() noexcept;
        void hud_render_get_tag_for_background() noexcept;

        TagID tag_id_for_ctf = HaloID::null_id();
        TagID tag_id_for_slayer = HaloID::null_id();
        TagID tag_id_for_oddball = HaloID::null_id();
        TagID tag_id_for_king = HaloID::null_id();
        TagID tag_id_for_race = HaloID::null_id();
        TagID tag_id_for_bg = HaloID::null_id();

        bool hud_render_draw_gametype = false;
    }

    static bool gametype_indicator_rendered_this_frame = false;
    bool gametype_indicator_disabled = false;

    void reset_gametype_indicator_rendered_this_frame() noexcept {
        gametype_indicator_rendered_this_frame = false;
    }

    extern "C" void hud_render_gametype_draw_this_frame() noexcept {
        if(gametype_indicator_rendered_this_frame || gametype_indicator_disabled) {
            hud_render_draw_gametype = false;
        }
        else {
            hud_render_draw_gametype = true;
            gametype_indicator_rendered_this_frame = true;
        }
    }

    void get_gametype_tags_for_new_map() noexcept {
        Tag *tag = get_tag("ui\\shell\\bitmaps\\team_background", TagClassInt::TAG_CLASS_BITMAP);
        tag_id_for_bg = tag ? tag->id : HaloID::null_id();

        tag = get_tag("ui\\shell\\bitmaps\\team_icon_ctf", TagClassInt::TAG_CLASS_BITMAP);
        tag_id_for_ctf = tag ? tag->id : HaloID::null_id();

        tag = get_tag("ui\\shell\\bitmaps\\team_icon_slayer", TagClassInt::TAG_CLASS_BITMAP);
        tag_id_for_slayer = tag ? tag->id : HaloID::null_id();

        tag = get_tag("ui\\shell\\bitmaps\\team_icon_oddball", TagClassInt::TAG_CLASS_BITMAP);
        tag_id_for_oddball = tag ? tag->id : HaloID::null_id();

        tag = get_tag("ui\\shell\\bitmaps\\team_icon_king", TagClassInt::TAG_CLASS_BITMAP);
        tag_id_for_king = tag ? tag->id : HaloID::null_id();

        tag = get_tag("ui\\shell\\bitmaps\\team_icon_race", TagClassInt::TAG_CLASS_BITMAP);
        tag_id_for_race = tag ? tag->id : HaloID::null_id();
    }

    void set_up_gametype_indicator_fix() noexcept {
        static Hook gametype_hook;
        skip_gametype_draw_ptr = get_chimera().get_signature("gametype_skip_draw_sig").data();
        write_function_override(get_chimera().get_signature("gametype_draw_sig").data(), gametype_hook, reinterpret_cast<const void *>(hud_render_gametype_draw_this_frame_asm), &original_gametype_draw_instr);

        static Hook hook0, hook1, hook2, hook3, hook4, hook5;
        SigByte nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

        auto *bg_sig = get_chimera().get_signature("team_icon_background_name_sig").data() + 14;
        write_code_s(bg_sig, nop);
        write_jmp_call(bg_sig, hook0, nullptr, reinterpret_cast<const void *>(hud_render_get_tag_for_background), false);

        auto *ctf_sig = get_chimera().get_signature("team_icon_ctf_sig").data();
        write_code_s(ctf_sig, nop);
        write_jmp_call(ctf_sig, hook1, nullptr, reinterpret_cast<const void *>(hud_render_get_tag_for_ctf), false);

        auto *slayer_sig = get_chimera().get_signature("team_icon_slayer_sig").data();
        write_code_s(slayer_sig, nop);
        write_jmp_call(slayer_sig, hook2, nullptr, reinterpret_cast<const void *>(hud_render_get_tag_for_slayer), false);

        auto *oddball_sig = get_chimera().get_signature("team_icon_oddball_sig").data();
        write_code_s(oddball_sig, nop);
        write_jmp_call(oddball_sig, hook3, nullptr, reinterpret_cast<const void *>(hud_render_get_tag_for_oddball), false);

        auto *king_sig = get_chimera().get_signature("team_icon_king_sig").data();
        write_code_s(king_sig, nop);
        write_jmp_call(king_sig, hook4, nullptr, reinterpret_cast<const void *>(hud_render_get_tag_for_king), false);

        auto *race_sig = get_chimera().get_signature("team_icon_race_sig").data();
        write_code_s(race_sig, nop);
        write_jmp_call(race_sig, hook5, nullptr, reinterpret_cast<const void *>(hud_render_get_tag_for_race), false);

        add_map_load_event(get_gametype_tags_for_new_map);
        add_frame_event(reset_gametype_indicator_rendered_this_frame);
    }
}
