// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <chrono>
#include <cstring>
#include <locale>
#include <codecvt>
#include <cwchar>
#include <cctype>
#include <regex>
#include "../event/frame.hpp"
#include "../event/tick.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/draw_text.hpp"
#include "../output/output.hpp"
#include "../chimera.hpp"
#include "../halo_data/player.hpp"
#include "../halo_data/server.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../config/ini.hpp"
#include "../localization/localization.hpp"
#include "../console/console.hpp"
#include "../halo_data/chat.hpp"
#include "emoji_map.hpp"

extern "C" void on_multiplayer_message(const wchar_t *message);
extern "C" void on_chat_message(const wchar_t *message);
extern "C" void on_chat_button(int channel);
extern "C" void on_kill_feed(wchar_t *a, std::uint32_t b, wchar_t *c);
const void *kill_feed_message;

namespace Chimera {
    static void on_custom_chat_frame() noexcept;
    static void on_chat_input() noexcept;
    static void enable_input(bool enabled) noexcept;
    static bool custom_chat_initialized = false;
    static const char *color_id_for_player(std::uint8_t player, ColorARGB *color_to_use);
    static void check_for_quit_players();
    static void load_chat_settings();

    static std::wstring u8_to_u16(const char *str) {
        wchar_t strw[1024] = {};
        if(MultiByteToWideChar(CP_UTF8, 0, str, -1, strw, sizeof(strw) / sizeof(*strw)) == 0) {
            return std::wstring();
        }
        else {
            return std::wstring(strw);
        }
    }

    static std::string u16_to_u8(const wchar_t *strw) {
        char str[1024] = {};
        if(WideCharToMultiByte(CP_UTF8, 0, strw, -1, str, sizeof(str) / sizeof(*str), nullptr, nullptr) == 0) {
            return std::string();
        }
        else {
            return std::string(str);
        }
    }

    extern "C" std::uint32_t chat_get_local_rcon_id() noexcept {
        auto *list = ServerInfoPlayerList::get_server_info_player_list();
        auto *player = PlayerTable::get_player_table().get_client_player();
        if(!list || !player) {
            return 0xFFFFFFFF;
        }
        auto *local_player = list->get_player(player->get_full_id());
        if(local_player) {
            return local_player - list->players;
        }
        else {
            return 0xFFFFFFFF;
        }
    }

    // Initialize the stuff
    void initialize_custom_chat() noexcept {
        // First, make this function do nothing
        const SigByte goodbye_code[] = { 0xC3, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(get_chimera().get_signature("multiplayer_message_sig").data(), goodbye_code);

        // Make the multiplayer hook
        static Hook chat_hook;
        write_jmp_call(get_chimera().get_signature("on_multiplayer_chat_sig").data(), chat_hook, nullptr, reinterpret_cast<const void *>(on_chat_message));

        // Make the hook for pressing the chat key
        static Hook chat_key_hook;
        const void *old_fn;
        write_function_override(get_chimera().get_signature("chat_open_sig").data(), chat_key_hook, reinterpret_cast<const void *>(on_chat_button), &old_fn);

        // Make the hook for pressing the chat key
        static Hook key_press_hook;
        write_jmp_call(get_chimera().get_signature("on_key_press_sig").data(), key_press_hook, nullptr, reinterpret_cast<const void *>(on_chat_input));

        add_preframe_event(on_custom_chat_frame);
        add_pretick_event(check_for_quit_players);

        // Make the hook for handling kill feed things
        static Hook kill_feed_hook;
        write_function_override(get_chimera().get_signature("kill_feed_sig").data(), kill_feed_hook, reinterpret_cast<const void *>(on_kill_feed), &kill_feed_message);

        // Set up the kill feed stuff
        auto &hud_kill_feed_sig = get_chimera().get_signature("hud_kill_feed_sig");
        auto &hud_kill_feed_host_kill_sig = get_chimera().get_signature("hud_kill_feed_host_kill_sig");
        auto &hud_kill_feed_host_betray_sig = get_chimera().get_signature("hud_kill_feed_host_betray_sig");
        overwrite(hud_kill_feed_sig.data() + 1, reinterpret_cast<int>(static_cast<void (*)(const wchar_t *)>(hud_output_raw)) - reinterpret_cast<int>(hud_kill_feed_sig.data() + 5));
        overwrite(hud_kill_feed_host_kill_sig.data() + 1, reinterpret_cast<int>(static_cast<void (*)(const wchar_t *)>(hud_output_raw)) - reinterpret_cast<int>(hud_kill_feed_host_kill_sig.data() + 5));
        overwrite(hud_kill_feed_host_betray_sig.data() + 1, reinterpret_cast<int>(static_cast<void (*)(const wchar_t *)>(hud_output_raw)) - reinterpret_cast<int>(hud_kill_feed_host_betray_sig.data() + 5));

        load_chat_settings();

        custom_chat_initialized = true;
    }

    bool custom_chat_enabled() noexcept {
        return custom_chat_initialized;
    }

    typedef std::chrono::high_resolution_clock clock;

    static float seconds_since_time(const clock::time_point &time) noexcept {
        auto duration = clock::now() - time;
        return static_cast<float>(duration.count()) / static_cast<float>(decltype(duration)::period::den);
    }

    #define MAX_MESSAGE_LENGTH 256
    struct ChatMessage {
        /** Message text */
        wchar_t message[MAX_MESSAGE_LENGTH];

        /** Just for padding */
        std::uint32_t zero = 0;

        /** Time message was created */
        clock::time_point created;

        /** Color to use */
        ColorARGB color = {1.0, 1.0, 1.0, 1.0};

        /**
         * Return true if valid
         * @return true if valid
         */
        bool valid() {
            return *this->message;
        }

        /**
         * Return time in seconds since message creation
         * @return time in seconds since message creation
         */
        float time_since_creation() {
            return seconds_since_time(created);
        }
    };

    static void add_message_to_array(ChatMessage *array, const ChatMessage &message);

    #define MESSAGE_BUFFER_SIZE 256
    static ChatMessage server_messages[MESSAGE_BUFFER_SIZE] = {};
    static ChatMessage chat_messages[MESSAGE_BUFFER_SIZE] = {};

    ////////////////////////////////////////////////////////////////////////////
    // Server messages
    ////////////////////////////////////////////////////////////////////////////
    static ColorARGB server_message_color = {0.5F, 0.45F, 0.72F, 1.0F};
    static std::uint16_t server_message_x = 400;
    static std::uint16_t server_message_y = 60;
    static std::uint16_t server_message_w = 390;
    static std::uint16_t server_message_h = 380;
    static std::uint16_t server_message_h_chat_open = 160;
    static TextAnchor server_message_anchor = TextAnchor::ANCHOR_TOP_RIGHT;
    static bool server_message_hide_on_console = false;
    static GenericFont server_message_font = GenericFont::FONT_SMALL;
    static float server_message_line_height = 1.1F;

    ////////////////////////////////////////////////////////////////////////////
    // Chat messages
    ////////////////////////////////////////////////////////////////////////////
    static ColorARGB chat_message_color_ffa = {0.85, 0.85, 1.0, 0.85};
    static ColorARGB chat_message_color_red = {0.85, 1.0, 0.85, 0.85};
    static ColorARGB chat_message_color_blue = {0.85, 0.85, 0.85, 1.0};
    static std::uint16_t chat_message_x = 8;
    static std::uint16_t chat_message_y = 38;
    static std::uint16_t chat_message_w = 612;
    static std::uint16_t chat_message_h = 300;
    static std::uint16_t chat_message_h_chat_open = 300;
    static TextAnchor chat_message_anchor = TextAnchor::ANCHOR_TOP_LEFT;
    static bool chat_message_hide_on_console = true;
    static GenericFont chat_message_font = GenericFont::FONT_SMALL;
    static float chat_message_line_height = 1.1F;

    ////////////////////////////////////////////////////////////////////////////
    // Chat input
    ////////////////////////////////////////////////////////////////////////////
    static ColorARGB chat_input_color = {1.0, 1.0, 1.0, 1.0};
    static std::uint16_t chat_input_x = 8;
    static std::uint16_t chat_input_y = 338;
    static std::uint16_t chat_input_w = 612;
    static TextAnchor chat_input_anchor = TextAnchor::ANCHOR_TOP_LEFT;
    static GenericFont chat_input_font = GenericFont::FONT_SMALL;

    static float chat_slide_time_length = 0.25F;
    static float server_slide_time_length = 0.0F;
    static float chat_time_up = 5.0F;
    static float server_time_up = 5.0F;
    static float chat_fade_out_time = 1.0F;
    static float server_fade_out_time = 1.0F;

    static std::size_t chat_message_scroll = 0;
    static bool block_ips = false;

    #define INPUT_BUFFER_SIZE 64
    static std::string chat_input_buffer;
    static std::size_t chat_input_cursor = 0;
    static int chat_input_channel = 0;
    static bool chat_input_open = false;
    static clock::time_point chat_open_state_changed;

    static bool player_in_server[16] = {};
    static char player_name[16][64];

    static bool show_chat_color_help = false;
    void set_show_color_help(bool show_help) noexcept {
        show_chat_color_help = show_help;
    }
    bool show_color_help() noexcept {
        return show_chat_color_help;
    }

    static void on_custom_chat_frame() noexcept {
        auto handle_messages = [](auto array, auto x, auto y, auto w, auto h, auto anchor, bool ignore_age, std::size_t scroll, GenericFont font, float slide_time_length, float time_up, float fade_out_time, float line_height_multiplier) {
            // Define the font
            std::uint16_t line_height = font_pixel_height(font) * line_height_multiplier;
            if(line_height == 0) {
                line_height = 1;
            }

            // Calculate the offset for the first (bottom) line
            std::uint16_t y_offset = y + h;

            // Find out how many lines we can have. Either it'll be do to pixel space requirements (most likely) or due to the maximum buffer size
            std::size_t max_lines = MESSAGE_BUFFER_SIZE - 1;
            std::size_t max_lines_height = h / line_height;
            std::size_t max_lines_actual = max_lines;
            if(max_lines_actual > max_lines_height) {
                max_lines_actual = max_lines_height;
            }

            // Find out if we have a brand new one
            float first_time_alive = array[0].time_since_creation();
            std::size_t new_count = 0;

            if(first_time_alive < slide_time_length) {
                // Find how many are also new
                new_count = 1;
                for(std::size_t i = 1; i < max_lines; i++) {
                    if(array[i].time_since_creation() < slide_time_length) {
                        new_count++;
                    }
                    else {
                        break;
                    }
                }
            }

            // Figure out how many we're going through
            std::size_t max_lines_iteration = max_lines_actual + new_count;
            if(max_lines_iteration > max_lines) {
                max_lines_iteration = max_lines;
            }

            if(scroll == 0) {
                // Increase the y value by how many new lines are coming in multiplied by the slide timing, but only if we're not scrolling
                y_offset += (new_count * line_height) - (slide_time_length == 0.0F ? 1.0F : first_time_alive / slide_time_length) * (new_count * line_height);
            }

            for(std::size_t i = scroll; i < max_lines_iteration + scroll; i++) {
                if(i >= max_lines) {
                    break;
                }

                if(!array[i].valid()) {
                    continue;
                }

                // Check if this is too old
                float time_alive = array[i].time_since_creation();
                if(!ignore_age && time_alive >= time_up) {
                    continue;
                }

                // Anyway, make the opacity based on whether it's fading out or in
                ColorARGB color = array[i].color;
                if(!ignore_age && time_alive > time_up - fade_out_time) {
                    color.alpha *= 1.0 - (time_alive - (time_up - fade_out_time)) / fade_out_time;
                }
                if(time_alive < slide_time_length) {
                    color.alpha *= slide_time_length == 0.0F ? 1.0F : time_alive / slide_time_length;
                }

                // Print the damn thing
                if(color.alpha > 0.0) {
                    apply_text_quake_colors(std::wstring(array[i].message), x, y_offset, w, line_height, color, font, anchor);
                }

                y_offset -= line_height;
            }
        };

        bool console_is_open = get_console_open();

        if((!console_is_open || !server_message_hide_on_console) && !server_messages_blocked()) {
            handle_messages(server_messages, server_message_x, server_message_y, server_message_w, chat_input_open ? server_message_h_chat_open : server_message_h, server_message_anchor, chat_input_open, 0, server_message_font, server_slide_time_length, server_time_up, server_fade_out_time, server_message_line_height);
        }
        if(!console_is_open || !chat_message_hide_on_console) {
            handle_messages(chat_messages, chat_message_x, chat_message_y, chat_message_w, chat_input_open ? chat_message_h_chat_open : chat_message_h, chat_message_anchor, chat_input_open, chat_message_scroll, chat_message_font, chat_slide_time_length, chat_time_up, chat_fade_out_time, chat_message_line_height);
        }

        // Handle chat input
        if(chat_input_open) {
            char prompt_prefix[INPUT_BUFFER_SIZE * 2] = {};
            const char *channel_name;
            if(chat_input_channel == 0) {
                channel_name = "chimera_custom_chat_to_all";
            }
            else if(chat_input_channel == 1) {
                channel_name = "chimera_custom_chat_to_team";
            }
            else {
                channel_name = "chimera_custom_chat_to_vehicle";
            }

            // Define the font for chat input
            std::uint16_t line_height = font_pixel_height(chat_input_font);
            std::size_t adjusted_y = chat_input_y + line_height * 2;
            if(line_height == 0) {
                line_height = 1;
            }

            // Draw the prompt
            std::snprintf(prompt_prefix, sizeof(prompt_prefix), "%s - ", localize(channel_name));
            auto x_offset_text_buffer = text_pixel_length(prompt_prefix, chat_input_font);
            apply_text_quake_colors(prompt_prefix, chat_input_x, adjusted_y, chat_input_w, line_height, chat_input_color, chat_input_font, chat_input_anchor);

            // Draw the entered text
            auto u16_chat_buffer = u8_to_u16(chat_input_buffer.c_str());
            apply_text_quake_colors(u16_chat_buffer, chat_input_x + x_offset_text_buffer, adjusted_y, chat_input_w, line_height, chat_input_color, chat_input_font, chat_input_anchor);

            // Figure out where and what color to draw the cursor
            const static std::regex color_code_re = std::regex("\\^(?:\\^|(.))");
            auto pre_cursor_text = chat_input_buffer.substr(0, chat_input_cursor);

            // Strip all color codes from text (saving the last-encountered one to use to render the
            // cursor with) in order to get an accurate length of the text before the cursor.
            std::string cursor_color = "^;";
            unsigned int pos = 0;
            auto colorless_pre_cursor_text = std::string();
            colorless_pre_cursor_text.reserve(pre_cursor_text.length());
            for(std::sregex_iterator i = std::sregex_iterator(pre_cursor_text.begin(), pre_cursor_text.end(), color_code_re); i != std::sregex_iterator(); i++){
                auto prev_len = i->position() - pos;
                colorless_pre_cursor_text.append(pre_cursor_text, pos, prev_len);
                if (i->length(1) > 0){
                    // color code - remember it but don't add it to the string
                    cursor_color = i->str();
                }
                else{
                    // matched a "^^" - add a literal "^"
                    colorless_pre_cursor_text.append("^");
                }
                pos += prev_len + i->length();
            }
            colorless_pre_cursor_text.append(pre_cursor_text, pos);

            // if the cursor is the middle of a color code definition then pop off the trailing "^" before computing the length
            if (chat_input_cursor < chat_input_buffer.length() && pos != pre_cursor_text.length() && colorless_pre_cursor_text.back() == '^' && chat_input_buffer[chat_input_cursor+1] != '^'){
                colorless_pre_cursor_text.pop_back();
            }

            // get the width of the color-code-less version of the buffer and draw the cursor there
            long cursor_x = text_pixel_length(u8_to_u16(colorless_pre_cursor_text.c_str()).c_str(), chat_input_font);
            cursor_color.append("_");
            apply_text_quake_colors(u8_to_u16(cursor_color.c_str()), cursor_x + chat_input_x + x_offset_text_buffer, adjusted_y, chat_input_w, line_height, chat_input_color, chat_input_font, chat_input_anchor);

            if(show_chat_color_help) {
                const char *color_codes = "1234567890\nqwertyuiop QWERTYUIOP\nasdfghjkl ASDFGHJKL\nzxcvbnm ZXCVBNM";
                std::size_t help_y = adjusted_y + line_height;
                std::size_t help_x = chat_input_x;
                for(const char *code = color_codes; *code; code++) {
                    if(*code == '\n') {
                        help_y += line_height;
                        help_x = chat_input_x;
                    }
                    else if(*code != ' ') {
                        char code_text[] = {'^', *code, '^', '^', *code, 0};
                        apply_text_quake_colors(code_text, help_x, help_y, chat_input_w, line_height, chat_input_color, chat_input_font, chat_input_anchor);
                    }
                    char code_space[] = {'^', *code, 0};
                    help_x += text_pixel_length(code_space, chat_input_font);
                }
            }
        }
    }

    static void initialize_chat_message(ChatMessage &message, const char *message_text, const ColorARGB &color);

    // Determine what color ID to use for a player
    static const char *color_id_for_player(std::uint8_t player, ColorARGB *color_to_use) {
        auto &player_info = ServerInfoPlayerList::get_server_info_player_list()->players[player];
        std::uint8_t armor_color = player_info.armor_color;
        std::uint8_t team = player_info.team;

        // Correct these values (SAPP's st command may have memed them)
        auto *player_table_player = player_info.get_player_table_player();
        if(player_table_player) {
            team = player_table_player->team;
        }

        const char *name_color_to_use = ";";
        if(is_team()) {
            if(team == 0) {
                name_color_to_use = "<";
                if(color_to_use) {
                    *color_to_use = chat_message_color_red;
                }
            }
            else {
                name_color_to_use = ">";
                if(color_to_use) {
                    *color_to_use = chat_message_color_blue;
                }
            }
        }
        else {
            static const char *armor_colors[] = {
                "m", // white
                "0", // black
                "1", // red
                "4", // blue
                "9", // gray
                "3", // yellow
                "V", // green
                "J", // pink
                "H", // purple
                "5", // cyan
                "F", // cobalt
                "u", // orange
                "B", // teal
                "S", // sage
                "O", // brown
                "o", // tan
                "I", // maroon
                "K"  // salmon
            };
            if(armor_color >= sizeof(armor_colors) / sizeof(*armor_colors)) {
                armor_color = 0;
            }
            name_color_to_use = armor_colors[armor_color];
            if(color_to_use) {
                *color_to_use = chat_message_color_ffa;
            }
        }

        return name_color_to_use;
    }

    extern "C" void draw_chat_message(const wchar_t *message, std::uint32_t p_int, std::uint32_t c_int) {

        // Ignore invalid channels
        const std::uint8_t channel_index = static_cast<std::uint8_t>(c_int);
        if(channel_index > 3) {
            return;
        }

        std::size_t length = lstrlenW(message);

        // apply IP removal if the message is long enough to have an IP in it
        std::wstring message_filtered;
        if(block_ips && length > 6) {
			// initialize once
            const static std::wregex r(L"\\b(\\d{1,3}\\.){3}\\d{1,3}\\b");
            message_filtered = std::regex_replace(message, r, L"#.#.#.#");
            message = message_filtered.c_str();
            length = lstrlenW(message);
        }

        ChatMessage chat_message;
        const std::uint8_t player_index = static_cast<std::uint8_t>(p_int);

        // This is a server message. No need to format.
        if(channel_index == 3 || player_index > 15) {

            char s[256] = {};
            if(length >= sizeof(s)) {
                length = 255;
            }

            // Next, let's see if we can break it up into lines
            std::size_t pos = 0;
            std::size_t s_len = 0;
            std::size_t last_space = 0;
            for(; pos < length; pos++) {
                s[s_len] = message[pos];
                s[s_len + 1] = 0;
                if(s[s_len] == ' ') {
                    last_space = s_len;
                }
                if(text_pixel_length(s, server_message_font) > server_message_w) {
                    if(last_space) {
                        s[last_space] = 0;
                        pos -= (s_len - last_space);
                        s_len = 0;
                        last_space = 0;
                    }
                    else {
                        s[s_len] = 0;
                        s_len = 0;
                        pos--;
                    }

                    initialize_chat_message(chat_message, s, server_message_color);
                    add_message_to_array(server_messages, chat_message);
                    continue;
                }
                s_len++;
            }
            if(s_len) {
                initialize_chat_message(chat_message, s, server_message_color);
                add_message_to_array(server_messages, chat_message);
            }
            return;
        }

        // Find the correct format to use
        const char *format_to_use;
        switch(channel_index) {
            case 1:
                format_to_use = localize("chimera_custom_chat_from_team");
                break;
            case 2:
                format_to_use = localize("chimera_custom_chat_from_vehicle");
                break;
            default:
                format_to_use = localize("chimera_custom_chat_from_all");
                break;
        }

        // Get color info
        ColorARGB color_to_use;
        auto *name_color_to_use = color_id_for_player(player_index, &color_to_use);

        // Re-encode from UTF-16 to UTF-8, as wcrtomb is producing unreliable results
        std::string message_u8 = u16_to_u8(message);

        // Format a message
        char entire_message[MAX_MESSAGE_LENGTH];
        std::snprintf(entire_message, sizeof(entire_message) - 1, format_to_use, name_color_to_use, player_name[player_index], message_u8.c_str());

        // Initialize; any UTF-8 codepoints that were cut off by the format will be replaced by
        // U+FFFD (or other appropriate character) as detailed in the documentation for MultiByteToWideChar.
        initialize_chat_message(chat_message, entire_message, color_to_use);
        add_message_to_array(chat_messages, chat_message);

        if(chat_message_scroll && chat_message_scroll + 1 != MESSAGE_BUFFER_SIZE) {
            chat_message_scroll++;
        }
    }

    void add_server_message(const char *message) {
        draw_chat_message(u8_to_u16(message).c_str(), 255, 3);
    }

    static void initialize_chat_message(ChatMessage &message, const wchar_t *message_text, const ColorARGB &color) {
        message.created = clock::now();
        message.color = color;

        // Zero it out
        std::memset(message.message, 0, sizeof(message.message));

        // Copy it over
        std::size_t max_size = lstrlenW(message_text);
        std::size_t limit = sizeof(message.message) / sizeof(*message.message) - 1;
        if(max_size > limit) {
            max_size = limit;
        }
        std::copy(message_text, message_text + max_size, message.message);
    }

    static void initialize_chat_message(ChatMessage &message, const char *message_text, const ColorARGB &color) {
        initialize_chat_message(message, u8_to_u16(message_text).c_str(), color);
    }

    static void add_message_to_array(ChatMessage *array, const ChatMessage &message) {
        for(std::size_t i = MESSAGE_BUFFER_SIZE - 1; i > 0; i--) {
            array[i] = array[i - 1];
        }
        *array = message;
    }

    extern "C" void bring_up_chat_prompt(int channel) {
        if(chat_input_open) {
            return;
        }
        chat_input_open = true;
        chat_input_buffer = std::string();
        chat_input_cursor = 0;
        chat_input_channel = channel;
        chat_open_state_changed = clock::now();
        enable_input(false);
    }

    static std::vector<unsigned int> get_char_start_idxs(unsigned int num_bytes){
        // Calculate an array of character start indexes. These can sometimes be
        // multiple characters visually. Zero width joining characters are
        // supported.
        std::vector<unsigned int> ret;
        static const unsigned int MASK = 0b11000000;
        static const unsigned int START = 0b11000000;
        static const unsigned int CONT = 0b10000000;
        static const unsigned int ZWJ[] = {0b11100010, 0b10000000, 0b10001101};

        unsigned int idx = 0;
        unsigned int zwjidx = 0;
        bool zwjing = false;
        for (unsigned int i = 0; i < num_bytes; i++){

            // Look for the ZWJ character
            if ((chat_input_buffer[i] & 0b11111111) == ZWJ[zwjidx]){
                zwjidx++;
                if (zwjidx == sizeof(ZWJ)/sizeof(ZWJ[0])){
                    zwjidx = 0;
                    idx--; // remove the starting index of the ZWJ character
                    zwjing = true;
                }
            }
            else {
                zwjidx = 0;
            }

            auto masked = chat_input_buffer[i] & MASK;
            if (masked == START || masked != CONT){
                // start of a multibyte char or just a normal char
                if (!zwjing){
                    // previous character was not a ZWJ - add the current char's index
                    ret.push_back(i);
                }
                zwjing = false;
            }
        }
        ret.push_back(num_bytes);
        return ret;
    }

    static unsigned int next_cursor_pos(std::vector<unsigned int> char_starts, int step){
        unsigned int num_chars = char_starts.size();
        unsigned int char_idx = 0;

        // find current character index
        for (unsigned int i = 0; i < num_chars; i++){
            if (char_starts[i] >= chat_input_cursor){
                char_idx = i;
                if (char_starts[i] > chat_input_cursor && step > 0){
                    // The cursor is (somehow) in the middle of a multi-byte character and we missed
                    // the correct index. This shouldn't happen but handle it anyway by reducing the
                    // number of characters to move right.
                    step -= 1;
                }
                break;
            }
        }
        // apply step and limits
        char_idx = std::min(std::max(0, int(char_idx) + step), int(num_chars)-1);

        return char_starts[char_idx];
    }

    static void on_chat_input() noexcept {
        struct key_input {
            std::uint8_t modifier; // 0001=shift 0010=ctrl 0100=alt
            std::uint8_t character;
            std::uint8_t key_code;
            std::uint8_t unknown; // definitely set to different values but meaning is unclear
        }; static_assert(sizeof(key_input) == sizeof(std::uint32_t)); // 4-byte strides

        static key_input    *input_buffer = nullptr; // array of size 0x40
        static std::int16_t *input_count = nullptr;  // population count for input_buffer
        if(!input_buffer) {
            auto *data = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("on_key_press_sig").data() + 10);
            input_buffer = reinterpret_cast<key_input*>(data + 2);
            input_count = reinterpret_cast<std::int16_t*>(data);
        }

        // Handle keyboard input if we have the chat input open
        if(chat_input_open) {
            const auto& [modifier, character, key_code, input_unknown] = input_buffer[*input_count];
            auto num_bytes = chat_input_buffer.length();

            // Special key pressed
            if(character == 0xFF) {
                bool ctrl  = modifier & 0b0000010;
                auto char_starts = get_char_start_idxs(num_bytes);

                if(key_code == 0) {
                    chat_input_open = false;
                    chat_open_state_changed = clock::now();
                    chat_message_scroll = 0;
                    enable_input(true);
                }
                // Up arrow / Page up
                else if(key_code == 0x4D || key_code == 0x53) {
                    if(chat_message_scroll + 1 != MESSAGE_BUFFER_SIZE && chat_messages[chat_message_scroll + 1].valid()) {
                        chat_message_scroll++;
                    }
                }
                // Down arrow / Page down
                else if(key_code == 0x4E || key_code == 0x56) {
                    if(chat_message_scroll > 0) {
                        chat_message_scroll--;
                    }
                }
                // Home
                else if(key_code == 0x52){
                    chat_input_cursor = 0;
                }
                // End
                else if(key_code == 0x55){
                    chat_input_cursor = num_bytes;
                }
                // Left arrow
                else if(key_code == 0x4F) {
                    if (ctrl){
                        // Ctrl + Left: move to start of previous word
                        while (chat_input_cursor > 0 && chat_input_buffer[chat_input_cursor-1] == ' '){
                            chat_input_cursor = next_cursor_pos(char_starts, -1);
                        }
                        if (chat_input_cursor > 0){
                            auto new_pos = chat_input_buffer.rfind(' ', chat_input_cursor-1);
                            chat_input_cursor = new_pos != std::string::npos ? new_pos + 1 : 0;
                        }
                    }
                    else{
                        chat_input_cursor = next_cursor_pos(char_starts, -1);
                    }
                }
                // Right arrow
                else if(key_code == 0x50) {
                    if (ctrl){
                        // Ctrl + Right: move to start of next word
                        if (chat_input_cursor < num_bytes){
                            auto new_pos = chat_input_buffer.find(' ', chat_input_cursor);
                            chat_input_cursor = new_pos != std::string::npos ? new_pos + 1 : num_bytes;
                        }
                        while (chat_input_cursor < num_bytes && chat_input_buffer[chat_input_cursor] == ' '){
                            chat_input_cursor = next_cursor_pos(char_starts, 1);
                        }
                    }
                    else{
                        chat_input_cursor = next_cursor_pos(char_starts, 1);
                    }
                }
                // Backspace
                else if(key_code == 0x1D) {
                    auto new_pos = next_cursor_pos(char_starts, -1);
                    chat_input_buffer.erase(new_pos, chat_input_cursor - new_pos);
                    chat_input_cursor = new_pos;
                }
                // Delete
                else if(key_code == 0x54) {
                    auto next_pos = next_cursor_pos(char_starts, 1);
                    chat_input_buffer.erase(chat_input_cursor, next_pos - chat_input_cursor);
                }
                // Enter
                else if(key_code == 0x38) {
                    if(num_bytes > 0 && server_type() != ServerType::SERVER_NONE){
                        chat_out(chat_input_channel, chat_input_buffer.c_str());
                    }
                    chat_input_open = false;
                    chat_open_state_changed = clock::now();
                    chat_message_scroll = 0;
                    enable_input(true);
                }
            }
            // typed a non-control character and there's room left in the buffer
            else if (!std::iscntrl(character) && num_bytes < INPUT_BUFFER_SIZE - 1) {
                bool inserted_emoji = false;

                // check for possible emoji replacement when typing ':'
                if (chat_input_cursor >= 2 && character == ':'){
                    auto start = chat_input_buffer.rfind(':', chat_input_cursor - 1);
                    if (start != std::string::npos && chat_input_cursor - start > 1) {
                        unsigned int name_len = chat_input_cursor - start - 1;
                        auto emoji_name = chat_input_buffer.substr(start + 1, name_len);
                        try {
                            // get the emoji from the name (raises exception if not found)
                            auto emoji = EMOJI_MAP.at(emoji_name);

                            // found an emoji, insert it if there's enough space in the buffer
                            unsigned int emoji_len = emoji.length();
                            int added_bytes = emoji_len - (name_len + 1);
                            if (num_bytes + added_bytes < INPUT_BUFFER_SIZE){
                                chat_input_buffer.erase(start, name_len + 1);
                                chat_input_buffer.insert(start, emoji);
                                chat_input_cursor += added_bytes;
                                inserted_emoji = true;
                            }
                        }
                        catch (const std::out_of_range& oor) {
                            // no match, insert the character normally
                        }
                    }
                }
                if (!inserted_emoji) {
                    // Insert the character normally
                    if(character >= 0x80) {
                        // Not enough space
                        if(num_bytes >= INPUT_BUFFER_SIZE - 2) {
                            return;
                        }

                        // Needs to be converted to UTF-8
                        chat_input_buffer.insert(chat_input_cursor++, 1, 0xC2 + (character > 0xBF ? 1 : 0));
                        chat_input_buffer.insert(chat_input_cursor++, 1, 0x80 + (character & 0x3F));
                    }
                    else {
                        // Can be used as-is
                        chat_input_buffer.insert(chat_input_cursor++, 1, character);
                    }

                }
            }
        }
    }

    static void enable_input(bool enabled) noexcept {
        auto &sig = get_chimera().get_signature("key_press_mov_sig");
        overwrite(sig.data() + 6, static_cast<std::uint8_t>(enabled));
    }

    template<typename T> static void strip_color_codes(T *str_data) {
        for(;*str_data;str_data++) {
            if(*str_data == '^') {
                std::size_t deletion_count = 2;
                if(str_data[1] == '^') {
                    deletion_count = 1;
                }
                T *str;
                for(str = str_data; str[deletion_count]; str++) {
                    str[0] = str[deletion_count];
                }
                *str = 0;
            }
        }
    }

    std::wstring get_string_from_string_list(const char *tag, std::uint32_t index) {
        auto *ustr = get_tag(tag, TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        if(!ustr) {
            return std::wstring();
        }

        auto *ustr_tag_data = ustr->data;
        std::uint32_t strings_count = *reinterpret_cast<std::uint32_t *>(ustr_tag_data);
        if(index >= strings_count) {
            return std::wstring();
        }
        auto *strings = *reinterpret_cast<std::byte **>(ustr_tag_data + 0x4);
        auto *str_ref = strings + 0x14 * index;
        auto str_len = *reinterpret_cast<std::size_t *>(str_ref + 0x0);
        auto *str = *reinterpret_cast<wchar_t **>(str_ref + 0xC);

        // Truncate down to this
        return std::wstring(str, str_len);
    }

    extern "C" void welcome_message(PlayerID player_a) {
        // Get server information
        auto *server_info = ServerInfoPlayerList::get_server_info_player_list();

        // Get the player name
        auto get_player_name = [&server_info](PlayerID id, ColorARGB *color_to_use) -> std::string {
            auto *player = server_info->get_player(id);
            return std::string("^") + color_id_for_player(player - server_info->players, color_to_use) + u16_to_u8(player->name).c_str() + "^;";
        };

        // Show a single message
        auto single_message = [&get_player_name](PlayerID id, int index) {
            auto str = u16_to_u8(get_string_from_string_list("ui\\multiplayer_game_text", index).c_str());
            ColorARGB color_to_use = {1.0, 1.0, 1.0, 1.0};

            std::string player_name = get_player_name(id, &color_to_use);

            char message[256];
            std::snprintf(message, sizeof(message), str.c_str(), player_name.c_str());

            ChatMessage chat_message;
            initialize_chat_message(chat_message, message, color_to_use);
            add_message_to_array(chat_messages, chat_message);
        };

        // Copy the name if we need to
        if(!player_in_server[player_a.index.index]) {
            auto *player_table_player = PlayerTable::get_player_table().get_player(player_a);
            single_message(player_a, 74);
            player_in_server[player_a.index.index] = true;

            auto *u8_name_hold = player_name[player_a.index.index];
            auto u8_player_name = u16_to_u8(player_table_player->name);
            std::strcpy(u8_name_hold, u8_player_name.c_str());
            char *t;
            for(t = player_name[player_a.index.index]; *t; t++);
            t[0] = '^';
            t[1] = ';';
            t[2] = 0;

            strip_color_codes(player_table_player->name);
        }
    }

    static void check_for_quit_players() {
        auto *server_info = ServerInfoPlayerList::get_server_info_player_list();
        if(!server_info) {
            std::fill(player_in_server, player_in_server + sizeof(player_in_server), false);
            return;
        }

        char message[256];

        for(bool &s : player_in_server) {
            if(!s) {
                continue;
            }
            std::size_t index = &s - player_in_server;
            if(server_info->players[index].player_id == 0xFF) {
                s = false;
                ChatMessage chat_message;
                auto s = u16_to_u8(get_string_from_string_list("ui\\multiplayer_game_text", 80).c_str());
                std::snprintf(message, sizeof(message), s.c_str(), player_name[index]);
                initialize_chat_message(chat_message, message, {1.0, 1.0, 1.0, 1.0});
                add_message_to_array(chat_messages, chat_message);
            }
        }
    }

    bool ips_blocked() noexcept {
        return block_ips;
    }
    void set_block_ips(bool block) noexcept {
        block_ips = block;
    }

    static void load_chat_settings() {
        auto *ini = get_chimera().get_ini();

        #define LOAD_IF_POSSIBLE(ini_value, setting, parser) { \
            const char *setting_value = ini->get_value("custom_chat." # ini_value); \
            if(setting_value) { \
                setting = parser(setting_value); \
            } \
        }

        #define LOAD_IF_POSSIBLE_SETTING(setting, parser) LOAD_IF_POSSIBLE(setting, setting, parser)

        LOAD_IF_POSSIBLE_SETTING(server_message_x, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(server_message_y, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(server_message_w, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(server_message_h, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(server_message_h_chat_open, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(server_message_hide_on_console, std::stoi)

        LOAD_IF_POSSIBLE_SETTING(chat_message_x, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(chat_message_y, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(chat_message_w, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(chat_message_h, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(chat_message_h_chat_open, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(chat_message_hide_on_console, std::stoi)

        LOAD_IF_POSSIBLE_SETTING(chat_input_x, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(chat_input_y, std::stoi)
        LOAD_IF_POSSIBLE_SETTING(chat_input_w, std::stoi)

        LOAD_IF_POSSIBLE(server_line_height, server_message_line_height, std::stof)
        LOAD_IF_POSSIBLE(chat_line_height, chat_message_line_height, std::stof)

        LOAD_IF_POSSIBLE(server_message_color_a, server_message_color.alpha, std::stof)
        LOAD_IF_POSSIBLE(server_message_color_r, server_message_color.red, std::stof)
        LOAD_IF_POSSIBLE(server_message_color_g, server_message_color.green, std::stof)
        LOAD_IF_POSSIBLE(server_message_color_b, server_message_color.blue, std::stof)

        LOAD_IF_POSSIBLE(chat_message_color_ffa_a, chat_message_color_ffa.alpha, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_ffa_r, chat_message_color_ffa.red, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_ffa_g, chat_message_color_ffa.green, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_ffa_b, chat_message_color_ffa.blue, std::stof)

        LOAD_IF_POSSIBLE(chat_message_color_red_a, chat_message_color_red.alpha, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_red_r, chat_message_color_red.red, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_red_g, chat_message_color_red.green, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_red_b, chat_message_color_red.blue, std::stof)

        LOAD_IF_POSSIBLE(chat_message_color_blue_a, chat_message_color_blue.alpha, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_blue_r, chat_message_color_blue.red, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_blue_g, chat_message_color_blue.green, std::stof)
        LOAD_IF_POSSIBLE(chat_message_color_blue_b, chat_message_color_blue.blue, std::stof)

        LOAD_IF_POSSIBLE(chat_input_color_a, chat_input_color.alpha, std::stof)
        LOAD_IF_POSSIBLE(chat_input_color_r, chat_input_color.red, std::stof)
        LOAD_IF_POSSIBLE(chat_input_color_g, chat_input_color.green, std::stof)
        LOAD_IF_POSSIBLE(chat_input_color_b, chat_input_color.blue, std::stof)

        auto get_anchor = [](const char *anchor) {
            if(std::strcmp(anchor, "top_right") == 0) {
                return TextAnchor::ANCHOR_TOP_RIGHT;
            }
            else if(std::strcmp(anchor, "top_left") == 0) {
                return TextAnchor::ANCHOR_TOP_LEFT;
            }
            else if(std::strcmp(anchor, "center") == 0) {
                return TextAnchor::ANCHOR_CENTER;
            }
            else if(std::strcmp(anchor, "bottom_right") == 0) {
                return TextAnchor::ANCHOR_BOTTOM_RIGHT;
            }
            else if(std::strcmp(anchor, "bottom_left") == 0) {
                return TextAnchor::ANCHOR_BOTTOM_LEFT;
            }
            return TextAnchor::ANCHOR_TOP_RIGHT;
        };

        LOAD_IF_POSSIBLE_SETTING(server_message_anchor, get_anchor)
        LOAD_IF_POSSIBLE_SETTING(chat_message_anchor, get_anchor)
        LOAD_IF_POSSIBLE_SETTING(chat_input_anchor, get_anchor)

        LOAD_IF_POSSIBLE_SETTING(server_message_font, generic_font_from_string)
        LOAD_IF_POSSIBLE_SETTING(chat_message_font, generic_font_from_string)
        LOAD_IF_POSSIBLE_SETTING(chat_input_font, generic_font_from_string)

        // Load these values
        LOAD_IF_POSSIBLE_SETTING(chat_slide_time_length, std::stof)
        LOAD_IF_POSSIBLE_SETTING(server_slide_time_length, std::stof)
        LOAD_IF_POSSIBLE_SETTING(chat_time_up, std::stof)
        LOAD_IF_POSSIBLE_SETTING(server_time_up, std::stof)
        LOAD_IF_POSSIBLE_SETTING(chat_fade_out_time, std::stof)
        LOAD_IF_POSSIBLE_SETTING(server_fade_out_time, std::stof)

        #define OVERRIDE_IF_POSSIBLE(setting, chat, server) { \
            std::optional<float> setting; \
            LOAD_IF_POSSIBLE_SETTING(setting, std::stof); \
            if(setting.has_value()) { \
                chat = *setting; \
                server = *setting; \
            } \
        }

        OVERRIDE_IF_POSSIBLE(slide_time_length, chat_slide_time_length, server_slide_time_length)
        OVERRIDE_IF_POSSIBLE(time_up, chat_time_up, server_time_up)
        OVERRIDE_IF_POSSIBLE(fade_out_time, chat_fade_out_time, server_fade_out_time)

        chat_slide_time_length = std::max(0.0F, chat_slide_time_length);
        server_slide_time_length = std::max(0.0F, server_slide_time_length);

        chat_time_up = std::max(0.0F, chat_time_up);
        server_time_up = std::max(0.0F, server_time_up);

        chat_fade_out_time = std::max(0.0F, chat_fade_out_time);
        server_fade_out_time = std::max(0.0F, server_fade_out_time);
    }
}
