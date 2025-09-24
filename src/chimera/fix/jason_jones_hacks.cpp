// SPDX-License-Identifier: GPL-3.0-only


#include "jason_jones_hacks.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/map_load.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/tag_class.hpp"
#include "../halo_data/game_engine.hpp"

namespace Chimera {

    static float highres_num_multipler = 0.5f;

    void jason_jones_numbers() noexcept {

        // Keep the memes to custom edition pls.
        if(game_engine() != GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            return;
        }

        highres_num_multipler = 0.5f;

        auto *globals_tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *interface_bitmaps = *reinterpret_cast<std::byte **>(globals_tag->data + 0x140 + 0x4);

        // Get the hud digits tag
        auto *tag = get_tag(*reinterpret_cast<TagID *>(interface_bitmaps + 0xB0 + 0xC));
        if(!tag) {
            return;
        }
        auto *tag_data = tag->data;

        // Get bitmap tag
        auto *bitmap_tag = get_tag(*reinterpret_cast<TagID *>(tag_data + 0xC));
        if(!bitmap_tag) {
            return;
        }

        // Get bitmap tag data
        auto *bitmap_tag_data = bitmap_tag->data;
        if(!*reinterpret_cast<std::uint32_t *>(bitmap_tag_data + 0x60)) {
            return;
        }

        // Get old data
        auto *bitmap = *reinterpret_cast<std::byte **>(bitmap_tag_data + 0x64);
        auto *maybe_dimensions = reinterpret_cast<std::uint16_t *>(bitmap + 0x4);
        auto *usage_flags = reinterpret_cast<std::uint16_t *>(bitmap_tag_data + 0x6);

        // If it's 2x size, set the 0.5 scale flag
        if(maybe_dimensions[0] == 256) {
            // Check the number spacing
            auto *bitmap_digit_width = reinterpret_cast<std::uint8_t *>(tag_data + 0x10);
            auto *screen_digit_width = reinterpret_cast<std::uint8_t *>(tag_data + 0x11);
            auto *x_offset = reinterpret_cast<std::uint8_t *>(tag_data + 0x12);
            auto *y_offset = reinterpret_cast<std::uint8_t *>(tag_data + 0x13);
            auto *point_width = reinterpret_cast<std::uint8_t *>(tag_data + 0x14);
            auto *colon_width = reinterpret_cast<std::uint8_t *>(tag_data + 0x15);

            if(*screen_digit_width == 18) {
                // These are almost certainly using highres numbers, so meme it to work again.
                highres_num_multipler = 1.0f;
                *usage_flags = *usage_flags | 1 << 4;

                // Set stock tag values
                *bitmap_digit_width = 12;
                *screen_digit_width = 9;
                *x_offset = 1;
                *y_offset = 0;
                *point_width = 6;
                *colon_width = 6;
            }
        }
    }

    void jason_jones_sniper_ticks() noexcept {
        // Search for weapon_hud_interface tags
        auto &tag_data_header = get_tag_data_header();
        auto *tags = tag_data_header.tag_array;
        auto tag_count = tag_data_header.tag_count;
        for(std::uint32_t i = 0; i < tag_count; i++) {
            auto &tag = tags[i];
            if(tag.primary_class != TAG_CLASS_WEAPON_HUD_INTERFACE) {
                continue;
            }

            if(!tag.data) {
                continue;
            }

            auto *static_element_count = reinterpret_cast<std::uint32_t *>(tag.data + 0x60);
            // 3 static elements? Probably stock sniper interface tag.
            if(*static_element_count == 3) {
                auto *static_element_data = *reinterpret_cast<std::byte **>(tag.data + 0x64);

                auto *multitexture_count_0 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
                auto *anchor_x_0 = reinterpret_cast<short *>(static_element_data + 0x24);
                auto *anchor_y_0 = reinterpret_cast<short *>(static_element_data + 0x26);
                static_element_data += 0xB4;
                auto *multitexture_count_1 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
                auto *anchor_x_1 = reinterpret_cast<short *>(static_element_data + 0x24);
                auto *anchor_y_1 = reinterpret_cast<short *>(static_element_data + 0x26);
                static_element_data += 0xB4;
                auto *multitexture_count_2 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
                auto *anchor_x_2 = reinterpret_cast<short *>(static_element_data + 0x24);
                auto *anchor_y_2 = reinterpret_cast<short *>(static_element_data + 0x26);

                // Make sure it's the thing we want to bullshit.
                if(*multitexture_count_0 == 0 && *anchor_x_0 == 7 && *anchor_y_0 == 21 && *multitexture_count_1 == 1 && *anchor_x_1 == 92 &&
                        *anchor_y_1 == 85 && *multitexture_count_2 == 1 && *anchor_x_2 == 445 && *anchor_y_2 == 85) {

                    *anchor_x_1 = 132;
                    *anchor_y_1 = 113;
                    *anchor_x_2 = 484;
                    *anchor_y_2 = 113;
                }
            }
        }
    }

    void set_up_jason_jones_hacks() noexcept {
        add_map_load_event(jason_jones_sniper_ticks);

        // Uncomment to meme maps using the old refined 2x HUD numbers.
        //add_map_load_event(jason_jones_numbers);
        //overwrite(get_chimera().get_signature("hud_number_element_highres_sig").data() + 2, &highres_num_multipler);
    }
}
