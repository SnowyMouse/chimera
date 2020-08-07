// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include "../../../event/frame.hpp"
#include "../../../output/draw_text.hpp"
#include "../../../output/output.hpp"
#include "../../../halo_data/decal.hpp"
#include "../../../halo_data/effect.hpp"
#include "../../../halo_data/light.hpp"
#include "../../../halo_data/particle.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"
#include "../../../fix/extend_limits.hpp"

namespace Chimera {
    static void show_budget() noexcept;
    static bool use_stock_limits = false;

    bool budget_command(int argc, const char **argv) noexcept {
        static int setting = 0;
        if(argc) {
            remove_preframe_event(show_budget);
            int value = std::strtol(argv[0], nullptr, 10);
            use_stock_limits = value == 2;
            if(value == 1 || value == 2) {
                add_preframe_event(show_budget, EventPriority::EVENT_PRIORITY_FINAL);
            }
            else {
                value = 0;
            }
            setting = value;
        }
        console_output("%i", setting);
        return true;
    }

    static constexpr std::int16_t x_name = 400;
    static constexpr std::int16_t x_name_width = 150;

    static constexpr std::int16_t x_val = x_name - x_name_width;
    static constexpr std::int16_t x_val_width = 55;

    static constexpr std::int16_t x_max = x_val - x_val_width;
    static constexpr std::int16_t x_max_width = 55;

    static constexpr std::int16_t x_percent = x_max - x_max_width;
    static constexpr std::int16_t x_percent_width = 55;

    static ColorARGB blue = ColorARGB { 0.7, 0.45, 0.72, 1.0 };
    static ColorARGB yellow = ColorARGB { 0.7, 1.0, 1.0, 0.4 };
    static ColorARGB red = ColorARGB { 0.7, 1.0, 0.4, 0.4 };

    static void output_limit(const char *name, std::uint32_t value, std::uint32_t max, std::int16_t y, GenericFont font) noexcept;

    static void show_budget() noexcept {
        auto font = GenericFont::FONT_CONSOLE;
        std::int16_t increment = font_pixel_height(font);
        std::int16_t y = 480 - increment * 10;

        // Header
        apply_text("Type", x_name, y, x_name_width, 480, blue, font, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_TOP_RIGHT);
        apply_text("Count", x_val, y, x_val_width, 480, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
        if(use_stock_limits) {
            apply_text("Max*", x_max, y, x_max_width, 480, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
        }
        else {
            apply_text("Max", x_max, y, x_max_width, 480, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
        }
        apply_text("%", x_percent, y, x_percent_width, 480, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
        y += increment;

        // Objects
        auto &object_table = ObjectTable::get_object_table();
        output_limit("objects", object_table.count, use_stock_limits ? 2048 : object_table.max_elements, y, font);
        y += increment;

        // Visible Objects
        static auto *vis_object_addr = get_chimera().get_signature("visible_object_count_sig").data();
        auto visible_objects = **reinterpret_cast<std::uint16_t **>(vis_object_addr + 3);
        static auto *visible_object_limit_addr = get_chimera().get_signature("visible_object_limit_1_sig").data() + 7;
        auto max_visible_objects = use_stock_limits ? 256 : *reinterpret_cast<std::uint16_t *>(visible_object_limit_addr);
        output_limit("visible objects", visible_objects, max_visible_objects, y, font);
        y += increment;

        // BSP Polies
        static auto *bsp_polies_addr = get_chimera().get_signature("bsp_poly_count_sig").data();
        std::uint32_t bsp_polies;
        std::uint32_t max_bsp_polies;
        if(*reinterpret_cast<std::uint8_t *>(bsp_polies_addr + 0x6) == 0x90) {
            bsp_polies = **reinterpret_cast<std::uint32_t **>(bsp_polies_addr + 2);
             max_bsp_polies = use_stock_limits ? 16384 : BSP_POLY_LIMIT;
        }
        else {
            bsp_polies = **reinterpret_cast<std::uint32_t **>(bsp_polies_addr + 3);
            max_bsp_polies = use_stock_limits ? 16384 : *reinterpret_cast<std::uint16_t *>(bsp_polies_addr - 8);
        }
        output_limit("visible bsp polies", bsp_polies, max_bsp_polies, y, font);
        y += increment;

        // Decals
        auto &decal_table = DecalTable::get_decal_table();
        output_limit("decals", decal_table.count, use_stock_limits ? 2048 : decal_table.max_elements, y, font);
        y += increment;

        // Effects
        auto &effect_table = EffectTable::get_effect_table();
        output_limit("effects", effect_table.count, use_stock_limits ? 256 : effect_table.max_elements, y, font);
        y += increment;

        // Lights
        auto &light_table = LightTable::get_light_table();
        output_limit("lights", light_table.count, use_stock_limits ? 896 : light_table.max_elements, y, font);
        y += increment;

        // Particles
        auto &particle_table = ParticleTable::get_particle_table();
        output_limit("particles", particle_table.count, use_stock_limits ? 1024 : particle_table.max_elements, y, font);
        y += increment;

        if(use_stock_limits) {
            y += 5;
            apply_text("*stock max", x_max, y, x_max_width + x_percent_width, 480, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
        }
    }

    static void output_limit(const char *name, std::uint32_t value, std::uint32_t max, std::int16_t y, GenericFont font) noexcept {
        char buffer[64];
        auto color = blue;
        if(value >= max) {
            color = red;
        }
        else if(value >= max * 0.75) {
            color = yellow;
        }
        apply_text(name, x_name, y, x_name_width, 480, color, font, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_TOP_RIGHT);
        std::snprintf(buffer, sizeof(buffer), "%u", value);
        apply_text(buffer, x_val, y, x_val_width, 480, color, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
        std::snprintf(buffer, sizeof(buffer), "%u", max);
        apply_text(buffer, x_max, y, x_max_width, 480, color, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
        std::snprintf(buffer, sizeof(buffer), "%.02f", 100.0 * static_cast<double>(value) / static_cast<double>(max));
        apply_text(buffer, x_percent, y, x_percent_width, 480, color, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_TOP_RIGHT);
    }
}
