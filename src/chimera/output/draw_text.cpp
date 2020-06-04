// SPDX-License-Identifier: GPL-3.0-only

#include <variant>
#include "../halo_data/tag.hpp"
#include "../chimera.hpp"
#include "output.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "draw_text.hpp"

namespace Chimera {
    #include "color_codes.hpp"

    TagID &get_generic_font(GenericFont font) noexcept {
        // Get the globals tag
        auto *globals_tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *interface_bitmaps = *reinterpret_cast<std::byte **>(globals_tag->data + 0x144);

        // Console font is referenced here
        if(font == GenericFont::FONT_CONSOLE) {
            return *reinterpret_cast<TagID *>(interface_bitmaps + 0x10 + 0xC);
        }
        // System font
        else if(font == GenericFont::FONT_SYSTEM) {
            return *reinterpret_cast<TagID *>(interface_bitmaps + 0x00 + 0xC);
        }

        // Get HUD globals which has the remaining two fonts.
        auto *hud_globals = get_tag(*reinterpret_cast<TagID *>(interface_bitmaps + 0x60 + 0xC));
        if(font == GenericFont::FONT_LARGE) {
            return *reinterpret_cast<TagID *>(hud_globals->data + 0x48 + 0xC);
        }
        else {
            return *reinterpret_cast<TagID *>(hud_globals->data + 0x58 + 0xC);
        }
    }

    GenericFont generic_font_from_string(const char *str) noexcept {
        if(std::strcmp(str, "console") == 0) {
            return GenericFont::FONT_CONSOLE;
        }
        else if(std::strcmp(str, "system") == 0) {
            return GenericFont::FONT_SYSTEM;
        }
        else if(std::strcmp(str, "small") == 0) {
            return GenericFont::FONT_SMALL;
        }
        else if(std::strcmp(str, "large") == 0) {
            return GenericFont::FONT_LARGE;
        }
        return GenericFont::FONT_CONSOLE;
    }

    struct Text {
        // Text to display
        std::variant<std::string, std::wstring> text;

        // X coordinate
        std::int16_t x;

        // Y coordinate
        std::int16_t y;

        // Width of the box
        std::int16_t width;

        // Height of the box
        std::int16_t height;

        // Color of the text
        ColorARGB color;

        // Font to use
        TagID font;

        // Alignment of the font
        FontAlignment alignment;
    };

    static std::vector<Text> text_list;

    struct FontData {
        // Font being used
        TagID font;

        // I don't know
        std::uint32_t unknown1;

        // I still don't know
        std::int16_t unknown_ffff;

        // Alignment
        FontAlignment alignment;

        // Text to use
        ColorARGB color;
    };
    static_assert(sizeof(FontData) == 0x1C);

    static FontData *font_data;

    extern "C" void display_text(const void *data, std::uint32_t xy, std::uint32_t wh, const void *function_to_use);

    static void *draw_text_8_bit = nullptr;
    static void *draw_text_16_bit = nullptr;

    // This is called every frame, giving us a chance to add text
    static void on_text() {
        if(text_list.size() == 0) {
            return;
        }

        // TODO: SIGNATURE FOR FONT DATA
        auto old_font_data = *font_data;

        for(auto &text : text_list) {
            font_data->color = text.color;
            font_data->alignment = text.alignment;
            font_data->font = text.font;

            // Depending on if we're using 8-bit or 16-bit, draw stuff
            auto *u8 = std::get_if<std::string>(&text.text);
            auto *u16 = std::get_if<std::wstring>(&text.text);

            if(u8) {
                display_text(u8->data(), text.x * 0x10000 + text.y, text.width * 0x10000 + text.height, draw_text_8_bit);
            }
            else {
                display_text(u16->data(), text.x * 0x10000 + text.y, text.width * 0x10000 + text.height, draw_text_16_bit);
            }
        }
        *font_data = old_font_data;
        text_list.clear();
    }

    std::int16_t font_pixel_height(const TagID &font) noexcept {
        auto *tag = get_tag(font);
        auto *tag_data = tag->data;
        return *reinterpret_cast<std::uint16_t *>(tag_data + 0x4) + *reinterpret_cast<std::uint16_t *>(tag_data + 0x6);
    }

    template <typename C> static void get_dimensions_template(std::int32_t &width, std::int32_t &height, const C *text) {

    }

    template<typename T> std::int16_t text_pixel_length_t(const T *text, const TagID &font) {
        struct Character {
            std::uint16_t character;
            std::uint16_t character_width;
            char i_stopped_caring[16];
        };
        static_assert(sizeof(Character) == 0x14);

        // Get the tag
        auto *tag = get_tag(font);

        // If it's not loaded, don't care
        if(tag->indexed && reinterpret_cast<std::uintptr_t>(tag->data) < 65536) {
            return 0;
        }
        std::int16_t length = 0;

        auto tag_chars_count = *reinterpret_cast<std::uint32_t *>(tag->data + 0x7C);
        auto *tag_chars = *reinterpret_cast<Character **>(tag->data + 0x7C + 4);

        while(*text != 0) {
            auto old_length = length;

            int char_length = 0;
            for(std::size_t i = 0; i < tag_chars_count; i++) {
                bool same = false;
                if(sizeof(T) == 1) {
                    same = *reinterpret_cast<const std::uint8_t *>(text) == tag_chars[i].character;
                }
                else {
                    same = *text == tag_chars[i].character;
                }
                if(same) {
                    char_length = tag_chars[i].character_width;
                    break;
                }
            }

            if(char_length > 0) {
                length += char_length;

                // If overflow, no point continuing.
                if(old_length > length) {
                    return old_length;
                }
            }

            text++;
        }

        return length;
    }

    std::int16_t text_pixel_length(const char *text, const TagID &font) noexcept {
        return text_pixel_length_t(text, font);
    }

    std::int16_t text_pixel_length(const wchar_t *text, const TagID &font) noexcept {
        return text_pixel_length_t(text, font);
    }

    float widescreen_width_480p = 640.0;

    void apply_text(std::variant<std::string, std::wstring> text, std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, const ColorARGB &color, TagID font, FontAlignment alignment, TextAnchor anchor) noexcept {
        // Adjust the coordinates based on the given anchor
        switch(anchor) {
            case TextAnchor::ANCHOR_TOP_LEFT:
                break;
            case TextAnchor::ANCHOR_TOP_RIGHT:
                x = static_cast<std::int16_t>(widescreen_width_480p - x);
                break;
            case TextAnchor::ANCHOR_BOTTOM_RIGHT:
                x = static_cast<std::int16_t>(widescreen_width_480p - x);
                y = static_cast<std::int16_t>(480 - y);
                break;
            case TextAnchor::ANCHOR_BOTTOM_LEFT:
                y = static_cast<std::int16_t>(480 - y);
                break;
            case TextAnchor::ANCHOR_CENTER:
                y += 240;
                x += static_cast<std::int16_t>(widescreen_width_480p / 2.0f);
                break;
        }
        text_list.emplace_back(Text { text, x, y, static_cast<std::int16_t>(x + width), static_cast<std::int16_t>(y + height), color, font, alignment } );
    }

    template<class T> void apply_text_quake_colors_t(T text, std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, const ColorARGB &color, TagID font, TextAnchor anchor) {
        std::vector<std::tuple<char,T>> segments;

        // Adjust the base coordinates based on the given anchor
        switch(anchor) {
            case TextAnchor::ANCHOR_TOP_LEFT:
                break;
            case TextAnchor::ANCHOR_TOP_RIGHT:
                x = static_cast<std::int16_t>(widescreen_width_480p - x);
                break;
            case TextAnchor::ANCHOR_BOTTOM_RIGHT:
                x = static_cast<std::int16_t>(widescreen_width_480p - x);
                y = static_cast<std::int16_t>(480 - y);
                break;
            case TextAnchor::ANCHOR_BOTTOM_LEFT:
                y = static_cast<std::int16_t>(480 - y);
                break;
            case TextAnchor::ANCHOR_CENTER:
                y += 240;
                x += static_cast<std::int16_t>(widescreen_width_480p / 2.0f);
                break;
        }

        const auto *text_data = text.data();
        const auto *segment_start_position = text.data();
        bool last_char_was_caret = false;
        int current_color = 8;

        while(*text_data != 0) {
            // Check if this is a color code
            if(last_char_was_caret) {
                // Replace ^^ with ^ so ^ can be used in messages
                if(*text_data == '^') {
                    std::size_t length = text_data - text.data();
                    for(std::size_t i = length - 1; i < text.size(); i++) {
                        text[i] = text[i + 1];
                    }

                    text_data++;
                    last_char_was_caret = false;
                    continue;
                }

                const auto *last_char = text_data - 1;
                std::size_t length = last_char - segment_start_position;

                // If we do have a string, add it.
                if(length > 0) {
                    std::vector<typename T::value_type> substring(length + 1);
                    for(std::size_t i = 0; i < length; i++) {
                        substring[i] = segment_start_position[i];
                    }
                    substring[length] = 0;
                    segments.emplace_back(std::make_tuple(current_color, substring.data()));
                }

                // Record current color
                current_color = *text_data;
                segment_start_position = text_data + 1;

                last_char_was_caret = false;
            }
            else if(*text_data == '^') {
                last_char_was_caret = true;
            }

            text_data++;
        }

        // Add this last segment
        segments.push_back(std::make_tuple(current_color, T(segment_start_position)));

        for(auto &segment : segments) {
            auto color_int = std::get<char>(segment);
            auto &string = std::get<T>(segment);

            std::int16_t old_x = x;

            // Figure out what color is needed
            ColorARGB chosen_color = color;
            color_for_code(color_int, chosen_color);

            // Add the color to the list
            text_list.emplace_back(Text { string, x, y, static_cast<std::int16_t>(x + width), static_cast<std::int16_t>(y + height), chosen_color, font, FontAlignment::ALIGN_LEFT });

            // Offset, giving up if we're overflowing or exceed y
            x += text_pixel_length(string.data(), font);
            if(old_x > x) {
                break;
            }
        }
    }

    void apply_text_quake_colors(std::wstring text, std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, const ColorARGB &color, TagID font, TextAnchor anchor) noexcept {
        apply_text_quake_colors_t(text, x, y, width, height, color, font, anchor);
    }

    void apply_text_quake_colors(std::string text, std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, const ColorARGB &color, TagID font, TextAnchor anchor) noexcept {
        apply_text_quake_colors_t(text, x, y, width, height, color, font, anchor);
    }

    extern "C" {
        const void *draw_text_8_bit_original;
        const void *draw_text_16_bit_original;
        extern "C" void display_text_8_scaled() noexcept;
        extern "C" void display_text_16_scaled() noexcept;
    }

    void setup_text_hook() noexcept {
        static Hook hook;
        auto *text_hook_addr = get_chimera().get_signature("text_hook_sig").data();
        write_jmp_call(reinterpret_cast<void *>(text_hook_addr), hook, reinterpret_cast<const void *>(on_text));
        draw_text_8_bit = get_chimera().get_signature("draw_8_bit_text_sig").data();
        draw_text_16_bit = get_chimera().get_signature("draw_16_bit_text_sig").data();
        font_data = *reinterpret_cast<FontData **>(get_chimera().get_signature("text_font_data_sig").data() + 13);

        static Hook draw_scale_8, draw_scale_16;
        // void write_function_override(void *jmp_at, Hook &hook, const void *new_function, const void **original_function);
        write_function_override(draw_text_8_bit, draw_scale_8, reinterpret_cast<const void *>(display_text_8_scaled), &draw_text_8_bit_original);
        write_function_override(draw_text_16_bit, draw_scale_16, reinterpret_cast<const void *>(display_text_16_scaled), &draw_text_16_bit_original);
    }

    extern "C" void scale_halo_drawn_text(std::uint8_t *) noexcept {
    }

    extern "C" void unscale_halo_drawn_text() noexcept {
    }
}
