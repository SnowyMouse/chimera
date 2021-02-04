// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include <cmath>

#include "../math_trig/math_trig.hpp"
#include "../halo_data/pad.hpp"

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/tick.hpp"
#include "../output/output.hpp"
#include "../halo_data/hud_fonts.hpp"

#include "../halo_data/tag.hpp"
#include "../halo_data/resolution.hpp"
#include "../event/map_load.hpp"
#include "widescreen_fix.hpp"

extern "C" {
    void widescreen_element_reposition_hud() noexcept;
    const void *widescreen_element_position_hud_fn;

    void widescreen_element_reposition_multitexture_overlay() noexcept;
    const void *widescreen_element_position_multitexture_overlay_fn;

    void widescreen_element_upscale_hud() noexcept;
    const void *widescreen_element_position_hud_2_fn;

    std::uint32_t widescreen_upscale_nav_element = 0;
    void widescreen_set_upscale_flag() noexcept;
    void widescreen_unset_upscale_flag() noexcept;

    std::uint32_t widescreen_hud_no_center = 0;
    void widescreen_set_hud_no_center_flag() noexcept;
    void widescreen_unset_hud_no_center_flag() noexcept;

    void widescreen_element_reposition_menu() noexcept;
    const void *widescreen_element_position_menu_fn;

    void widescreen_element_reposition_motion_sensor() noexcept;
    const void *widescreen_element_position_motion_sensor_fn;

    void widescreen_element_reposition_letterbox() noexcept;
    const void *widescreen_element_position_letterbox_fn;

    void widescreen_element_reposition_menu_text() noexcept;
    const void *widescreen_element_position_menu_text_fn;

    void widescreen_element_reposition_menu_text_2() noexcept;
    const void *widescreen_element_position_menu_text_2_fn;

    void widescreen_element_reposition_text_f1() noexcept;
    const void *widescreen_element_position_text_f1_fn;

    void widescreen_element_reposition_text_pgcr() noexcept;
    const void *widescreen_element_position_text_pgcr_fn;

    void widescreen_element_reposition_text_stare_name() noexcept;
    const void *widescreen_element_position_text_stare_name_fn;

    void widescreen_element_reposition_text_f3_name() noexcept;
    const void *widescreen_element_position_text_f3_name_fn;

    void widescreen_element_reposition_text_cinematic() noexcept;
    const void *widescreen_element_position_text_cinematic_fn;

    void widescreen_input_text() noexcept;
    void widescreen_input_text_undo() noexcept;

    std::int16_t *widescreen_text_input_element;

    void widescreen_mouse() noexcept;
    std::int32_t widescreen_mouse_left_bounds = 0;
    std::int32_t widescreen_mouse_right_bounds = 640;
    std::int32_t *widescreen_mouse_x = nullptr;
    std::int32_t *widescreen_mouse_y = nullptr;

    std::int16_t widescreen_left_offset_add = 0;
    void widescreen_cutscene_text_before_asm() noexcept;
    void widescreen_cutscene_text_after_asm() noexcept;
}

namespace Chimera {
    WidescreenFixSetting setting = WidescreenFixSetting::WIDESCREEN_OFF;

    static void on_tick() noexcept;
    static void on_map_load() noexcept;
    static float *scope_width;
    static float aspect_ratio = 4.0f / 3.0f;
    extern float widescreen_width_480p;
    static float *hud_element_scaling;
    static float *text_scaling;
    static float *motion_sensor_scaling;
    static const constexpr float motion_sensor_x_offset_default = -1.001562476F;
    static float *motion_sensor_x_offset;
    static std::int32_t *console_width;
    static std::int32_t *text_max_x;
    static std::int16_t *f2_motd_x;
    static std::int16_t *f2_heading_x;
    static std::int16_t *f2_motd_body_x1;
    static std::int16_t *f2_motd_body_x2;
    static std::int16_t *f2_rules_1_x1;
    static std::int16_t *f2_rules_1_x2;
    static std::int16_t *f2_rules_2_x1;
    static std::int16_t *f2_rules_2_x2;
    static std::int16_t *f2_rules_3_x1;
    static std::int16_t *f2_rules_3_x2;
    static std::int16_t *f2_rules_4_x1;
    static std::int16_t *f2_rules_4_x2;
    static std::int32_t *f2_rules_4_left_x;
    static std::int16_t *loading_screen_text_x2;
    static std::int16_t *f1_server_name_x2;
    static std::int16_t *f1_server_ip_x2;
    static std::int32_t *console_output_width;
    static std::int16_t tabs[4];

    std::int16_t widescreen_text_x_offset = 0;

    enum Corner {
        CORNER_TOP_LEFT = 0,
        CORNER_TOP_RIGHT,
        CORNER_BOTTOM_RIGHT,
        CORNER_BOTTOM_LEFT
    };
    struct ElementCorner {
        float x;
        float y;
        PAD(0x8);
        float unknown[2];
    };
    static_assert(sizeof(ElementCorner) == 0x18);

    struct ElementData {
        ElementCorner corners[4];
    };

    extern "C" void reposition_hud_element(ElementData &element) noexcept {
        if(widescreen_hud_no_center) {
            return;
        }

        float increase_x = widescreen_width_480p - 640.000f;
        float increase_y = 0.0f;

        float min_x = element.corners[CORNER_TOP_LEFT].x;
        float max_x = element.corners[CORNER_TOP_RIGHT].x;
        float min_y = element.corners[CORNER_TOP_LEFT].y;
        float max_y = element.corners[CORNER_BOTTOM_LEFT].y;

        float center_x = (min_x + max_x) / 2.0f;
        float center_y = (min_y + max_y) / 2.0f;

        bool inner_center = std::fabs(320.0f - center_x) < 210.0 && std::fabs(240.0f - center_y) < 170.0;
        bool centered_x = inner_center || std::fabs(320.0f - center_x) < 100.0;
        //bool centered_y = inner_center || std::fabs(240.0f - center_y) < 100.0;

        // Center centered elements
        if(centered_x || setting == WidescreenFixSetting::WIDESCREEN_CENTER_HUD) {
            increase_x /= 2.0f;
        }
        else if(center_x < 320.0f) {
            increase_x = 0.0f;
        }

        // Adjust elements
        element.corners[0].x += increase_x;
        element.corners[1].x += increase_x;
        element.corners[2].x += increase_x;
        element.corners[3].x += increase_x;

        element.corners[0].y += increase_y;
        element.corners[1].y += increase_y;
        element.corners[2].y += increase_y;
        element.corners[3].y += increase_y;
    }

    extern "C" void upscale_hud_element(ElementData &element) noexcept {
        float screen_width = aspect_ratio * 480.000f;
        float center_x = (element.corners[CORNER_TOP_LEFT].x + element.corners[CORNER_TOP_RIGHT].x + element.corners[CORNER_BOTTOM_LEFT].x + element.corners[CORNER_BOTTOM_RIGHT].x) / 4.0f;
        float position_percentage = center_x / 640.0f;
        float new_center_x = position_percentage * screen_width;

        if(widescreen_upscale_nav_element > 0) {
            std::uint32_t n = widescreen_upscale_nav_element++;
            if(n > 1) {
                static ElementData last_element;
                static float last_center_x;
                static float last_width;

                float width = element.corners[CORNER_BOTTOM_RIGHT].x - element.corners[CORNER_BOTTOM_LEFT].x;

                if(n > 2) {
                    //float last_element_width = last_element.corners[CORNER_BOTTOM_LEFT].x - last_element.corners[CORNER_BOTTOM_RIGHT].x;
                    float spacing = last_element.corners[CORNER_BOTTOM_LEFT].x - element.corners[CORNER_BOTTOM_RIGHT].x;
                    new_center_x = last_center_x - last_width / 2.0 - spacing - width / 2.0;
                }
                else {
                    new_center_x -= 6 * (screen_width / 640.0 - 1) * (width / 2.0);
                }

                last_width = width;
                last_center_x = new_center_x;
                last_element = element;
            }
        }

        float delta = new_center_x - center_x;
        element.corners[CORNER_TOP_LEFT].x += delta;
        element.corners[CORNER_TOP_RIGHT].x += delta;
        element.corners[CORNER_BOTTOM_LEFT].x += delta;
        element.corners[CORNER_BOTTOM_RIGHT].x += delta;
    }

    extern "C" void reposition_menu_element(ElementData &element) noexcept {
        float increase = widescreen_width_480p - 640.000f;

        float min_x = element.corners[CORNER_TOP_LEFT].x;
        float max_x = element.corners[CORNER_TOP_RIGHT].x;

        float min_y = element.corners[CORNER_TOP_LEFT].y;
        float max_y = element.corners[CORNER_BOTTOM_LEFT].y;

        if(min_x <= 1.0f && max_x >= 639.0f && min_y <= 1.0F && max_y >= 479.0f) {
            element.corners[CORNER_TOP_RIGHT].x += increase;
            element.corners[CORNER_BOTTOM_RIGHT].x += increase;
            return;
        }
        else {
            element.corners[0].x += increase / 2.0f;
            element.corners[1].x += increase / 2.0f;
            element.corners[2].x += increase / 2.0f;
            element.corners[3].x += increase / 2.0f;
            return;
        }
    }

    extern "C" void reposition_letterbox_element(ElementData &element) noexcept {
        float increase = aspect_ratio * 480.000f - 640.000f;
        if(element.corners[CORNER_TOP_LEFT].x <= 1.0f && element.corners[CORNER_TOP_RIGHT].x >= 639.0f) {
            element.corners[CORNER_TOP_RIGHT].x += increase;
            element.corners[CORNER_BOTTOM_RIGHT].x += increase;
        }

        // This might not be a letterbox. It might also be a 2D box, so if so, we just move it to the right a bit.
        else {
            element.corners[CORNER_TOP_RIGHT].x += increase / 2;
            element.corners[CORNER_BOTTOM_RIGHT].x += increase / 2;
            element.corners[CORNER_TOP_LEFT].x += increase / 2;
            element.corners[CORNER_BOTTOM_LEFT].x += increase / 2;
        }
    }

    static std::uint16_t *tabs_ptr;

    extern "C" void reposition_menu_text_element(std::int16_t *element) noexcept {
        element[1] += widescreen_left_offset_add;
        element[3] += widescreen_left_offset_add;

        // This is for the score screens which use tabbing.
        std::uint16_t tab_count = static_cast<std::uint16_t>(tabs_ptr[0]);

        static std::int16_t last_increase = -1337;
        if(tab_count == 0) {
            last_increase = -1337;
        }
        else {
            if(tabs_ptr[1] != last_increase) {
                for(std::size_t i = 0; i < tab_count; i++) {
                    tabs_ptr[i + 1] += widescreen_left_offset_add;
                }
                last_increase = tabs_ptr[1];
            }
        }
    }

    extern "C" void reposition_menu_text_input() noexcept {
        std::int16_t increase = widescreen_left_offset_add;
        widescreen_text_input_element[1] += increase;
        widescreen_text_input_element[3] += increase;
    }

    extern "C" void unreposition_menu_text_input() noexcept {
        std::int16_t increase = widescreen_left_offset_add;
        widescreen_text_input_element[1] -= increase;
        widescreen_text_input_element[3] -= increase;
    }

    extern "C" void upscale_menu_text_element(std::int16_t *element) noexcept {
        // Basically this upscales the position of the element
        float screen_width = aspect_ratio * 480.000f;
        std::int16_t element_width = element[3] - element[1];
        std::int16_t element_center = (element[3] + element[1] + 1) / 2;
        float position_percentage = static_cast<float>(element_center) / 640.0f;
        std::int16_t new_element_center = static_cast<std::int16_t>(screen_width * position_percentage);
        element[1] = new_element_center - element_width / 2;
        element[3] = new_element_center + element_width / 2;
    }

    static void temporarily_unfix_scope_mask() noexcept;

    static bool ce = false;

    static bool f1 = false;

    bool widescreen_fix_enabled() noexcept {
        return setting;
    }

    void set_widescreen_fix(WidescreenFixSetting new_setting) noexcept {
        if(static_cast<bool>(new_setting) != static_cast<bool>(setting)) {
            bool demo = get_chimera().feature_present("client_demo");
            ce = get_chimera().feature_present("client_widescreen_custom_edition");
            tabs_ptr = *reinterpret_cast<std::uint16_t **>(get_chimera().get_signature("widescreen_text_tab_sig").data() + 0x3);
            f1 = get_chimera().feature_present("client_widescreen_f1");

            bool hud_text_mod = hud_text_mod_initialized();

            auto &widescreen_scope = get_chimera().get_signature("widescreen_scope_sig");
            scope_width = reinterpret_cast<float *>(widescreen_scope.data() + 4);

            auto &widescreen_element_scaling_sig = get_chimera().get_signature("widescreen_element_scaling_sig");
            hud_element_scaling = reinterpret_cast<float *>(widescreen_element_scaling_sig.data() + 7);
            
            static Hook position_hud;
            auto &widescreen_element_position_hud_sig = get_chimera().get_signature("widescreen_element_position_hud_sig");
            write_function_override(reinterpret_cast<void *>(widescreen_element_position_hud_sig.data()), position_hud, reinterpret_cast<const void *>(widescreen_element_reposition_hud), &widescreen_element_position_hud_fn);

            static Hook position_multitexture_overlay;
            auto &widescreen_element_position_multitexture_overlay_sig = get_chimera().get_signature("widescreen_element_position_multitexture_overlay_sig");
            write_function_override(reinterpret_cast<void *>(widescreen_element_position_multitexture_overlay_sig.data()), position_multitexture_overlay, reinterpret_cast<const void *>(widescreen_element_reposition_multitexture_overlay), &widescreen_element_position_multitexture_overlay_fn);

            static Hook position_menu;
            auto &widescreen_element_position_menu_sig = get_chimera().get_signature("widescreen_element_position_menu_sig");
            write_function_override(reinterpret_cast<void *>(widescreen_element_position_menu_sig.data()), position_menu, reinterpret_cast<const void *>(widescreen_element_reposition_menu), &widescreen_element_position_menu_fn);

            static Hook position_letterbox;
            auto &widescreen_element_position_letterbox_sig = get_chimera().get_signature("widescreen_element_position_letterbox_sig");
            write_function_override(reinterpret_cast<void *>(widescreen_element_position_letterbox_sig.data()), position_letterbox, reinterpret_cast<const void *>(widescreen_element_reposition_letterbox), &widescreen_element_position_letterbox_fn);

            auto &widescreen_text_scaling_sig = get_chimera().get_signature("widescreen_text_scaling_sig");
            text_scaling = reinterpret_cast<float *>(widescreen_text_scaling_sig.data() + 6);

            auto &widescreen_console_input_sig = get_chimera().get_signature("widescreen_console_input_sig");
            console_width = reinterpret_cast<std::int32_t *>(widescreen_console_input_sig.data() + 2);

            auto &widescreen_menu_text_sig = get_chimera().get_signature("widescreen_menu_text_sig");
            auto &widescreen_menu_text_2_sig = get_chimera().get_signature("widescreen_menu_text_2_sig");
            if(!hud_text_mod) {
                static Hook menu_text;
                write_function_override(reinterpret_cast<void *>(widescreen_menu_text_sig.data() + 9), menu_text, reinterpret_cast<const void *>(widescreen_element_reposition_menu_text), &widescreen_element_position_menu_text_fn);

                static Hook menu_text_2;
                write_function_override(reinterpret_cast<void *>(widescreen_menu_text_2_sig.data()), menu_text_2, reinterpret_cast<const void *>(widescreen_element_reposition_menu_text_2), &widescreen_element_position_menu_text_2_fn);
            }

            auto &widescreen_text_max_x_sig = get_chimera().get_signature("widescreen_text_max_x_sig");
            text_max_x = reinterpret_cast<std::int32_t *>(widescreen_text_max_x_sig.data() + 1);

            static Hook text_f1;
            auto &widescreen_text_f1_sig = get_chimera().get_signature(demo ? "widescreen_text_f1_demo_sig" : "widescreen_text_f1_sig");
            auto &widescreen_text_f1_server_ip_position_sig = get_chimera().get_signature("widescreen_text_f1_server_ip_position_sig");
            auto &widescreen_text_f1_server_name_position_sig = get_chimera().get_signature("widescreen_text_f1_server_name_position_sig");

            if(f1) {
                f1_server_ip_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_f1_server_ip_position_sig.data() + 5);
                f1_server_name_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_f1_server_name_position_sig.data() + 5);

                if(!hud_text_mod) {
                    write_function_override(reinterpret_cast<void *>(widescreen_text_f1_sig.data()), text_f1, reinterpret_cast<const void *>(widescreen_element_reposition_text_f1), &widescreen_element_position_text_f1_fn);
                }
            }

            static Hook text_pgcr;
            auto &widescreen_text_pgcr_sig = get_chimera().get_signature("widescreen_text_pgcr_sig");
            if(!hud_text_mod) {
                write_function_override(reinterpret_cast<void *>(widescreen_text_pgcr_sig.data()), text_pgcr, reinterpret_cast<const void *>(widescreen_element_reposition_text_pgcr), &widescreen_element_position_text_pgcr_fn);
            }

            auto &widescreen_element_motion_sensor_scaling_sig = get_chimera().get_signature("widescreen_element_motion_sensor_scaling_sig");
            motion_sensor_scaling = reinterpret_cast<float *>(widescreen_element_motion_sensor_scaling_sig.data() + 0x4);
            motion_sensor_x_offset = reinterpret_cast<float *>(widescreen_element_motion_sensor_scaling_sig.data() + 0x22);

            static Hook text_stare_name;
            auto &widescreen_text_stare_name_sig = get_chimera().get_signature("widescreen_text_stare_name_sig");
            if(!hud_text_mod) {
                write_function_override(reinterpret_cast<void *>(widescreen_text_stare_name_sig.data()), text_stare_name, reinterpret_cast<const void *>(widescreen_element_reposition_text_stare_name), &widescreen_element_position_text_stare_name_fn);
            }

            static Hook text_f3_name;
            if(ce && !hud_text_mod) {
                auto &widescreen_text_f3_name_sig = get_chimera().get_signature("widescreen_text_f3_name_sig");
                write_function_override(reinterpret_cast<void *>(widescreen_text_f3_name_sig.data()), text_f3_name, reinterpret_cast<const void *>(widescreen_element_reposition_text_f3_name), &widescreen_element_position_text_f3_name_fn);
            }

            static Hook nav_marker;
            auto &widescreen_nav_marker_sig = get_chimera().get_signature("widescreen_nav_marker_sig");
            write_jmp_call(widescreen_nav_marker_sig.data(), nav_marker, reinterpret_cast<const void *>(widescreen_set_upscale_flag), reinterpret_cast<const void *>(widescreen_unset_upscale_flag));

            static Hook nav_marker_sp;
            auto &widescreen_nav_marker_sp_sig = get_chimera().get_signature("widescreen_nav_marker_sp_sig");
            write_jmp_call(widescreen_nav_marker_sp_sig.data(), nav_marker_sp, reinterpret_cast<const void *>(widescreen_set_upscale_flag), reinterpret_cast<const void *>(widescreen_unset_upscale_flag));

            static Hook cutscene_text;
            auto &widescreen_text_cutscene_sig = get_chimera().get_signature("widescreen_text_cutscene_sig");
            if(!hud_text_mod) {
                write_jmp_call(widescreen_text_cutscene_sig.data() + 8, cutscene_text, reinterpret_cast<const void *>(widescreen_cutscene_text_before_asm), reinterpret_cast<const void *>(widescreen_cutscene_text_after_asm), false);
            }

            if(ce) {
                auto &widescreen_text_f2_text_position_motd_sig = get_chimera().get_signature("widescreen_text_f2_text_position_motd_sig");
                f2_motd_x = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_motd_sig.data() + 0xC);

                auto &widescreen_text_f2_text_position_heading_sig = get_chimera().get_signature("widescreen_text_f2_text_position_heading_sig");
                f2_heading_x = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_heading_sig.data() + 0x5);

                auto &widescreen_text_f2_text_position_motd_body_sig = get_chimera().get_signature("widescreen_text_f2_text_position_motd_body_sig");
                f2_motd_body_x1 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_motd_body_sig.data() + 0x5);
                f2_motd_body_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_motd_body_sig.data() + 0x7 + 0x5);

                auto &widescreen_text_f2_text_position_rules_1_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_1_sig");
                f2_rules_1_x1 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_1_sig.data() + 0x5);
                f2_rules_1_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_1_sig.data() + 0x7 + 0x5);

                auto &widescreen_text_f2_text_position_rules_2_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_2_sig");
                f2_rules_2_x1 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_2_sig.data() + 0x5);
                f2_rules_2_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_2_sig.data() + 0x7 + 0x5);

                auto &widescreen_text_f2_text_position_rules_3_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_3_sig");
                f2_rules_3_x1 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_3_sig.data() + 0x5);
                f2_rules_3_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_3_sig.data() + 0x7 + 0x5);

                auto &widescreen_text_f2_text_position_rules_4_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_4_sig");
                f2_rules_4_x1 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_4_sig.data() + 0x5);
                f2_rules_4_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_f2_text_position_rules_4_sig.data() + 0x7 + 0x5);

                auto &widescreen_text_f2_text_position_rules_4_left_x_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_4_left_x_sig");
                f2_rules_4_left_x = reinterpret_cast<std::int32_t *>(widescreen_text_f2_text_position_rules_4_left_x_sig.data() + 0x1);
            }

            static Hook position_teammate_indicator;
            auto &widescreen_teammate_indicator_sig = get_chimera().get_signature("widescreen_teammate_indicator_sig");
            write_function_override(reinterpret_cast<void *>(widescreen_teammate_indicator_sig.data()), position_teammate_indicator, reinterpret_cast<const void *>(widescreen_element_upscale_hud), &widescreen_element_position_hud_2_fn);

            static Hook pickup_icon;
            auto &widescreen_hud_pickup_icon_sig = get_chimera().get_signature("widescreen_hud_pickup_icon_sig");
            write_jmp_call(reinterpret_cast<void *>(widescreen_hud_pickup_icon_sig.data()), pickup_icon, reinterpret_cast<const void *>(widescreen_set_hud_no_center_flag), reinterpret_cast<const void *>(widescreen_unset_hud_no_center_flag));

            auto &widescreen_text_loading_screen_sig = get_chimera().get_signature("widescreen_text_loading_screen_sig");
            loading_screen_text_x2 = reinterpret_cast<std::int16_t *>(widescreen_text_loading_screen_sig.data() + 7 + 5);

            static Hook screen_effect;
            auto &widescreen_screen_effect_sig = get_chimera().get_signature("widescreen_screen_effect_sig");
            write_jmp_call(reinterpret_cast<void *>(widescreen_screen_effect_sig.data()), screen_effect, reinterpret_cast<const void *>(temporarily_unfix_scope_mask), reinterpret_cast<const void *>(temporarily_unfix_scope_mask));

            auto &widescreen_console_tabs_sig = get_chimera().get_signature("widescreen_console_tabs_sig");
            console_output_width = reinterpret_cast<std::int32_t *>(widescreen_console_tabs_sig.data() + 0x3A);
            overwrite(widescreen_console_tabs_sig.data() + 0x51 + 1, reinterpret_cast<std::int16_t *>(tabs));
            overwrite(widescreen_console_tabs_sig.data() + 0x56 + 3, reinterpret_cast<std::int16_t *>(tabs) + 2);

            static Hook input_text;
            auto &widescreen_input_text_sig = get_chimera().get_signature("widescreen_input_text_sig");
            if(!hud_text_mod) {
                write_jmp_call(reinterpret_cast<void *>(widescreen_input_text_sig.data()), input_text, reinterpret_cast<const void *>(widescreen_input_text), reinterpret_cast<const void *>(widescreen_input_text_undo));
            }

            static Hook widescreen_mouse_hook;
            const void *old_fn;
            auto &widescreen_mouse_sig = get_chimera().get_signature("widescreen_mouse_sig");
            widescreen_mouse_x = *reinterpret_cast<std::int32_t **>(widescreen_mouse_sig.data() + 4);
            widescreen_mouse_y = widescreen_mouse_x + 1;
            write_function_override(reinterpret_cast<void *>(widescreen_mouse_sig.data()), widescreen_mouse_hook, reinterpret_cast<const void *>(widescreen_mouse),&old_fn);

            if(new_setting) {
                add_tick_event(on_tick);
                add_map_load_event(on_map_load);
                on_map_load();
                on_tick();
            }
            else {
                remove_tick_event(on_tick);
                remove_map_load_event(on_map_load);

                widescreen_scope.rollback();
                widescreen_element_scaling_sig.rollback();
                widescreen_element_position_hud_sig.rollback();
                widescreen_element_position_multitexture_overlay_sig.rollback();
                widescreen_element_position_menu_sig.rollback();
                widescreen_element_position_letterbox_sig.rollback();
                widescreen_text_scaling_sig.rollback();
                widescreen_console_input_sig.rollback();
                if(!hud_text_mod) {
                    widescreen_menu_text_sig.rollback();
                    widescreen_menu_text_2_sig.rollback();
                    widescreen_text_stare_name_sig.rollback();
                    widescreen_text_pgcr_sig.rollback();
                    widescreen_text_cutscene_sig.rollback();
                }
                widescreen_text_max_x_sig.rollback();
                if(f1) {
                    if(!hud_text_mod) {
                        widescreen_text_f1_sig.rollback();
                    }
                    widescreen_text_f1_server_ip_position_sig.rollback();
                    widescreen_text_f1_server_name_position_sig.rollback();
                }
                widescreen_element_motion_sensor_scaling_sig.rollback();
                if(ce && !hud_text_mod) {
                    auto &widescreen_text_f3_name_sig = get_chimera().get_signature("widescreen_text_f3_name_sig");
                    widescreen_text_f3_name_sig.rollback();
                }
                widescreen_nav_marker_sig.rollback();
                widescreen_nav_marker_sp_sig.rollback();
                if(ce) {
                    auto &widescreen_text_f2_text_position_motd_sig = get_chimera().get_signature("widescreen_text_f2_text_position_motd_sig");
                    auto &widescreen_text_f2_text_position_heading_sig = get_chimera().get_signature("widescreen_text_f2_text_position_heading_sig");
                    auto &widescreen_text_f2_text_position_motd_body_sig = get_chimera().get_signature("widescreen_text_f2_text_position_motd_body_sig");
                    auto &widescreen_text_f2_text_position_rules_1_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_1_sig");
                    auto &widescreen_text_f2_text_position_rules_2_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_2_sig");
                    auto &widescreen_text_f2_text_position_rules_3_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_3_sig");
                    auto &widescreen_text_f2_text_position_rules_4_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_4_sig");
                    auto &widescreen_text_f2_text_position_rules_4_left_x_sig = get_chimera().get_signature("widescreen_text_f2_text_position_rules_4_left_x_sig");
                    widescreen_text_f2_text_position_motd_sig.rollback();
                    widescreen_text_f2_text_position_heading_sig.rollback();
                    widescreen_text_f2_text_position_motd_body_sig.rollback();
                    widescreen_text_f2_text_position_rules_1_sig.rollback();
                    widescreen_text_f2_text_position_rules_2_sig.rollback();
                    widescreen_text_f2_text_position_rules_3_sig.rollback();
                    widescreen_text_f2_text_position_rules_4_sig.rollback();
                    widescreen_text_f2_text_position_rules_4_left_x_sig.rollback();
                }
                widescreen_teammate_indicator_sig.rollback();
                widescreen_hud_pickup_icon_sig.rollback();
                widescreen_text_loading_screen_sig.rollback();
                widescreen_screen_effect_sig.rollback();
                widescreen_console_tabs_sig.rollback();
                if(!hud_text_mod) {
                    widescreen_input_text_sig.rollback();
                }
                widescreen_mouse_sig.rollback();

                widescreen_text_x_offset = 0;

                widescreen_width_480p = 640.0f;
                widescreen_mouse_left_bounds = 0;
                widescreen_mouse_right_bounds = 640;
            }
        }
        
        bool update_on_tick = setting != new_setting && setting;
        
        setting = new_setting;
        if(update_on_tick) {
            overwrite(console_width, 640);
            widescreen_width_480p = 640.0F;
            on_tick();
        }
    }

    extern void set_block_zoom_blur_disabled(bool disabled);

    static void temporarily_unfix_scope_mask() noexcept {
        static bool unfixed = false;
        if(unfixed) {
            overwrite(scope_width, widescreen_width_480p);
            unfixed = false;
            set_block_zoom_blur_disabled(false);
        }
        else {
            overwrite(scope_width, 640.0f);
            unfixed = true;
            set_block_zoom_blur_disabled(true);
        }
    }

    static void on_map_load() noexcept {
        // Make it so these things do NOT stretch
        auto jason_jones_tag = [](const char *tag_path) {
            auto *tag = get_tag(tag_path, TagClassInt::TAG_CLASS_UI_WIDGET_DEFINITION);
            if(!tag) {
                return;
            }
            std::int16_t &bounds_top = *reinterpret_cast<std::int16_t *>(tag->data + 0x24);
            std::int16_t &bounds_left = *reinterpret_cast<std::int16_t *>(tag->data + 0x26);
            std::int16_t &bounds_bottom = *reinterpret_cast<std::int16_t *>(tag->data + 0x28);
            std::int16_t &bounds_right = *reinterpret_cast<std::int16_t *>(tag->data + 0x2A);

            if(bounds_top == 0 && bounds_bottom == 480 && bounds_left == 0 && bounds_right == 640) {
                bounds_bottom = 478;
            }
        };
        jason_jones_tag("ui\\shell\\main_menu\\multiplayer_type_select\\join_game\\join_game_items_list");
        jason_jones_tag("ui\\shell\\main_menu\\settings_select\\player_setup\\player_profile_edit\\controls_setup\\controls_options_menu");
        jason_jones_tag("ui\\shell\\main_menu\\settings_select\\player_setup\\player_profile_edit\\gamepad_setup\\gamepad_setup_options");
    }

    static void on_tick() noexcept {
        aspect_ratio = static_cast<float>(get_resolution().width) / static_cast<float>(get_resolution().height);
        widescreen_left_offset_add = static_cast<std::int16_t>((aspect_ratio * 480.000f - 640.000f) / 2.0f);

        // Change instructions if we need them to be changed
        widescreen_width_480p = aspect_ratio * 480.0f;

        if(*console_width != static_cast<std::int32_t>(widescreen_width_480p)) {
            overwrite(scope_width, widescreen_width_480p);

            // Calculate mouse cursor bounds
            std::int32_t mouse_increase = (static_cast<std::int32_t>(widescreen_width_480p) - 640) / 2;
            widescreen_mouse_left_bounds = -mouse_increase;
            widescreen_mouse_right_bounds = 640 + mouse_increase;
            *widescreen_mouse_x = widescreen_mouse_left_bounds;

            float half_width_inverted = 2.0 / (aspect_ratio * 480.0f);
            overwrite(hud_element_scaling, half_width_inverted);
            overwrite(text_scaling, half_width_inverted);
            overwrite(motion_sensor_scaling, half_width_inverted);
            
            float motion_sensor_memes = motion_sensor_x_offset_default / (widescreen_width_480p / 640.0F);
            if(setting == WidescreenFixSetting::WIDESCREEN_CENTER_HUD) {
                overwrite(motion_sensor_x_offset, motion_sensor_memes);
            }
            else {
                overwrite(motion_sensor_x_offset, motion_sensor_x_offset_default);
            }

            overwrite(console_width, static_cast<std::int32_t>(widescreen_width_480p));
            overwrite(text_max_x, static_cast<std::uint32_t>(widescreen_width_480p));

            if(f1 && !hud_text_mod_initialized()) {
                overwrite(f1_server_ip_x2, static_cast<std::uint16_t>(widescreen_width_480p - 5));
                overwrite(f1_server_name_x2, static_cast<std::uint16_t>(widescreen_width_480p - 5));
            }

            if(ce) {
                overwrite(f2_motd_x, static_cast<std::int16_t>(widescreen_width_480p - 640.0f + 625));
                overwrite(f2_heading_x, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 630));
                overwrite(f2_motd_body_x1, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 105));
                overwrite(f2_motd_body_x2, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 625));
                overwrite(f2_rules_1_x1, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 330));
                overwrite(f2_rules_1_x2, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 630));
                overwrite(f2_rules_2_x1, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 10));
                overwrite(f2_rules_2_x2, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 310));
                overwrite(f2_rules_3_x1, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 330));
                overwrite(f2_rules_3_x2, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 630));
                overwrite(f2_rules_4_x1, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 310));
                overwrite(f2_rules_4_x2, static_cast<std::int16_t>((widescreen_width_480p - 640.0f) / 2 + 450));
                overwrite(f2_rules_4_left_x, static_cast<std::uint32_t>((widescreen_width_480p - 640.0f) / 2 + 10));
            }
            overwrite(loading_screen_text_x2, static_cast<std::int16_t>(widescreen_width_480p));
            overwrite(console_output_width, static_cast<std::int32_t>(widescreen_width_480p));

            tabs[0] = static_cast<std::int16_t>(0.25f * widescreen_width_480p);
            tabs[1] = static_cast<std::int16_t>(0.50f * widescreen_width_480p);
            tabs[2] = static_cast<std::int16_t>(0.75f * widescreen_width_480p);
        }
    }
}
