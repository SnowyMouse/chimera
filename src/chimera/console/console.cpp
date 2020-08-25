// SPDX-License-Identifier: GPL-3.0-only

#include "../localization/localization.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../command/command.hpp"
#include "../event/command.hpp"
#include "../event/tick.hpp"
#include "../event/frame.hpp"
#include "../output/output.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/resolution.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../config/ini.hpp"
#include "../fix/widescreen_fix.hpp"
#include "console.hpp"
#include "../output/draw_text.hpp"
#include "../halo_data/keyboard.hpp"

#include <fstream>
#include <deque>
#include <chrono>

namespace Chimera {
    static std::string rcon_password;
    static bool rcon_command_used_recently = false;
    static void read_command();
    static char *console_text = NULL;

    extern std::vector<Event<CommandEventFunction>> command_events;

    struct CommandEntry {
        std::uint32_t return_type; // 4 = server stuff
        const char *name;
        void *code_execute;
        void *code_execute_2;
        const char *help_text;
        const char *help_parameters;
        std::uint16_t more_stuff;
        std::uint16_t parameter_count;
        //std::uint32_t parameters[];
    };
    static_assert(sizeof(CommandEntry) == 0x1C);

    struct GlobalEntry {
        const char *name;
        std::uint32_t value_type;
        const void *default_value;
        std::uint32_t unknown;
    };
    static_assert(sizeof(GlobalEntry) == 0x10);

    static GlobalEntry ***entries_global = nullptr;
    static std::uint32_t *entry_count_global = nullptr;

    static CommandEntry ***entries = nullptr;
    static std::uint32_t *entry_count;
    static CommandEntry **old_entries;
    static std::uint32_t old_entry_count;

    static std::vector<CommandEntry *> new_entries_list;
    static std::vector<std::unique_ptr<CommandEntry>> new_entries_added;

    void script_command_dump_command(int, const char **) noexcept {
        char path[MAX_PATH];

        if(entries) {
            std::size_t command_entry_count = *entry_count;
            auto *command_entries = *entries;

            std::snprintf(path, sizeof(path), "%sscript_command_dump.json", get_chimera().get_path());

            std::ofstream o(path, std::ios_base::out | std::ios_base::trunc);
            o << "[\n";

            for(std::size_t i = 0; i < command_entry_count; i++) {
                o << "    { \"name\": \"" << command_entries[i]->name << "\", \"return\": " << command_entries[i]->return_type;
                const auto *parameters = reinterpret_cast<const std::uint16_t *>(command_entries[i] + 1);
                o << ", \"parameters\": [";
                for(std::size_t x = 0; x < command_entries[i]->parameter_count; x++) {
                    o << static_cast<std::uint16_t>(parameters[x]);
                    if(x + 1 != command_entries[i]->parameter_count) {
                        o << ",";
                    }
                }
                o << "] }";
                if(i + 1 != command_entry_count) {
                    o << ",";
                }
                o << "\n";
            }

            o << "]\n";
            o.flush();
            o.close();

            console_output("Dumped %zu command%s to %s", command_entry_count, command_entry_count == 1 ? "" : "s", path);
        }

        if(entries_global) {
            std::snprintf(path, sizeof(path), "%sscript_global_dump.json", get_chimera().get_path());
            std::ofstream o(path, std::ios_base::out | std::ios_base::trunc);
            o << "[\n";

            std::size_t global_entry_count = *entry_count_global;
            auto *global_entries = *entries_global;

            for(std::size_t i = 0; i < global_entry_count; i++) {
                o << "    { \"name\": \"" << global_entries[i]->name << "\", \"type\": " << global_entries[i]->value_type << ", \"value\": ";

                if(global_entries[i]->default_value) {
                    switch(global_entries[i]->value_type) {
                        case 5:
                            o << (*reinterpret_cast<const std::uint8_t *>(global_entries[i]->default_value) ? "true" : "false");
                            break;
                        case 6:
                            o << *reinterpret_cast<const float *>(global_entries[i]->default_value);
                            break;
                        case 7:
                            o << *reinterpret_cast<const std::int16_t *>(global_entries[i]->default_value);
                            break;
                        case 9:
                            o << "\"" << *reinterpret_cast<const char *>(global_entries[i]->default_value) << "\"";
                            break;
                        default:
                            o << *reinterpret_cast<const std::int32_t *>(global_entries[i]->default_value);
                            break;
                    }
                }
                else {
                    o << "null";
                }

                o << " }";
                if(i + 1 != global_entry_count) {
                    o << ",";
                }
                o << "\n";
            }

            o << "]\n";
            o.flush();
            o.close();

            console_output("Dumped %zu global%s to %s", global_entry_count, global_entry_count == 1 ? "" : "s", path);
        }
    }

    static void on_tab_completion_start() noexcept {
        auto &chimera_commands = get_chimera().get_commands();

        old_entry_count = *entry_count;
        old_entries = *entries;

        new_entries_list = std::vector<CommandEntry *>(old_entries, old_entries + old_entry_count);
        for(auto &command : chimera_commands) {
            if(!get_chimera().feature_present(command.feature())) {
                continue;
            }

            auto &new_command = new_entries_added.emplace_back(std::make_unique<CommandEntry>());
            new_command->return_type = 4;
            new_command->name = command.name();
            new_command->help_text = "see README.md";
            new_command->help_parameters = nullptr;
            new_command->more_stuff = 0x15;
            new_entries_list.emplace_back(new_command.get());
        }

        overwrite(entry_count, static_cast<std::uint32_t>(new_entries_list.size()));
        overwrite(entries, new_entries_list.data());
    }

    static void on_tab_completion_end() noexcept {
        overwrite(entry_count, old_entry_count);
        overwrite(entries, old_entries);

        new_entries_list.clear();
        new_entries_added.clear();
    }

    void initialize_console_hook() {
        static Hook hook;
        const auto &sig = get_chimera().get_signature("console_call_sig");
        write_jmp_call(sig.data(), hook, reinterpret_cast<const void *>(read_command));
        console_text = *reinterpret_cast<char **>(sig.data() - 4);

        static Hook on_tab_completion_hook;
        const auto on_tab_completion_sig = get_chimera().get_signature("on_tab_completion_sig");
        write_jmp_call(on_tab_completion_sig.data(), on_tab_completion_hook, reinterpret_cast<const void *>(on_tab_completion_start), reinterpret_cast<const void *>(on_tab_completion_end));

        bool non_custom = game_engine() != GameEngine::GAME_ENGINE_CUSTOM_EDITION;
        const char *sig_to_use;
        switch(game_engine()) {
            case GameEngine::GAME_ENGINE_CUSTOM_EDITION: {
                sig_to_use = "command_list_custom_edition_sig";
                auto *global_list_data = get_chimera().get_signature("global_list_custom_edition_sig").data();
                entries_global = reinterpret_cast<GlobalEntry ***>(global_list_data + 1);
                entry_count_global = reinterpret_cast<std::uint32_t *>(global_list_data + 6);
                break;
            }
            case GameEngine::GAME_ENGINE_RETAIL:
                sig_to_use = "command_list_retail_sig";
                break;
            case GameEngine::GAME_ENGINE_DEMO:
                sig_to_use = "command_list_demo_sig";
                break;
            default:
                std::terminate();
        }
        auto *command_list_data = get_chimera().get_signature(sig_to_use).data();
        entries = reinterpret_cast<CommandEntry ***>(command_list_data + 1);
        entry_count = reinterpret_cast<std::uint32_t *>(command_list_data + 5 + (non_custom ? 4 : 1));
    }

    // Disable Halo's error message that occurs when an invalid command is used.
    static void block_error() noexcept {
        static auto *where = get_chimera().get_signature("console_block_error_sig").data();
        const std::uint8_t nops_galore[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
        overwrite(where + 6, nops_galore, sizeof(nops_galore));
    }

    // Re-enable the error message.
    static void unblock_error() noexcept {
        get_chimera().get_signature("console_block_error_sig").rollback();
    }

    static void check_when_console_is_closed() noexcept;

    // This function intercepts the console and reads the buffer.
    static void read_command() {
        block_error();
        const Command *found_command;

        // If "clear" is sent to the console, change it to "cls"
        if(std::strcmp(console_text, "clear") == 0) {
            std::strcpy(console_text, "cls");
            return;
        }

        if (std::strcmp(console_text, "rs") == 0) { // Reload scripts shorthand command
            get_chimera().execute_command("chimera_reload_scripts");
            return;
        }

        // If this is the rcon command, we may need to do special things
        if(std::strncmp(console_text, "rcon", 4) == 0 && get_chimera().feature_present("client") && server_type() == ServerType::SERVER_DEDICATED) {
            rcon_command_used_recently = true;
            add_tick_event(check_when_console_is_closed);

            // If this is an rcon password, check if we're using rconp
            if(rcon_password.size() && std::strncmp(console_text, "rconp ", 6) == 0) {
                auto command_broken_up = split_arguments(console_text);
                command_broken_up[0] = "rcon";
                command_broken_up.insert(command_broken_up.begin() + 1, rcon_password);
                auto command_put_together = unsplit_arguments(command_broken_up);
                std::strncpy(console_text, command_put_together.data(), 75);
            }
        }

        switch(get_chimera().execute_command(console_text, &found_command, true)) {
            case CommandResult::COMMAND_RESULT_FAILED_ERROR_NOT_FOUND: {
                bool allow = true;
                call_in_order_allow(command_events, allow, console_text);
                if(allow) {
                    unblock_error();
                }
                break;
            }
            case CommandResult::COMMAND_RESULT_FAILED_NOT_ENOUGH_ARGUMENTS:
                console_error(localize("chimera_error_not_enough_arguments"), found_command->name(), found_command->min_args());
                break;
            case CommandResult::COMMAND_RESULT_FAILED_TOO_MANY_ARGUMENTS:
                console_error(localize("chimera_error_too_many_arguments"), found_command->name(), found_command->max_args());
                break;
            case CommandResult::COMMAND_RESULT_FAILED_FEATURE_NOT_AVAILABLE:
                console_error(localize("chimera_error_command_unavailable"), found_command->name(), found_command->feature());
                break;
            case CommandResult::COMMAND_RESULT_SUCCESS:
                break;
            default:
                break;
        }
    }

    void set_auto_suspend_on_console(bool suspend) noexcept {
        overwrite(get_chimera().get_signature("suspend_if_paused_sig").data(), static_cast<std::uint8_t>(suspend ? 0x74 : 0xEB));
    }

    static std::uint8_t *console_enabled() noexcept {
        static std::uint8_t *addr = nullptr;
        if(!addr) {
            addr = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("console_enabled_sig").data() + 1);
        }
        return addr;
    }

    void set_console_enabled(bool enabled) noexcept {
        *console_enabled() = enabled;
    }

    bool get_console_enabled() noexcept {
        return *console_enabled();
    }

    bool get_console_open() noexcept {
        static std::uint8_t *addr = nullptr;
        if(!addr) {
            addr = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("console_enabled_sig").data() + 1) - 1;
        }
        return *addr;
    }

    static const char *get_console_text() {
        static const char *addr = nullptr;
        if(!addr) {
            addr = *reinterpret_cast<const char **>(get_chimera().get_signature("console_buffer_sig").data() + 2);
        };
        return addr;
    }

    static void check_when_console_is_closed() noexcept {
        static std::uint8_t *addr = nullptr;
        if(!addr) {
            addr = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("console_enabled_sig").data() + 1) - 1;
        }
        if(!get_console_open()) {
            rcon_command_used_recently = false;
            remove_tick_event(check_when_console_is_closed);
        }
    }

    bool rcon_used_recently() noexcept {
        return rcon_command_used_recently;
    }

    extern "C" void fade_out_console_asm();

    static void fade_out_console() {
        if(!get_console_open()) {
            fade_out_console_asm();
        }
    }

    void setup_console_fade_fix() noexcept {
        overwrite(get_chimera().get_signature("console_fade_call_sig").data(), static_cast<std::uint8_t>(0xEB));
        add_pretick_event(fade_out_console);
    }

    using SteadyClock = std::chrono::steady_clock;

    struct Line {
        std::string text;
        SteadyClock::time_point created;
        ColorARGB color;
    };
    static std::deque<Line> custom_lines;
    static std::size_t max_lines;
    static std::size_t max_lines_soft;
    static std::size_t position = 0;
    static bool use_scrollback = false;
    static std::size_t more_lines_below = 0;

    extern "C" void override_console_output_eax_asm();
    extern "C" void override_console_output_edi_asm();

    static double fade_start;
    static double fade_time;
    static double total_lifetime;
    #define MICROSECONDS_PER_SEC 1000000
    static double line_height;
    static int x_margin;

    static void on_console_frame() {
        auto font = GenericFont::FONT_CONSOLE;
        auto height = font_pixel_height(font);
        auto this_line_height = height * line_height;
        if(this_line_height < 1) {
            this_line_height = 1;
        }
        auto open = get_console_open();

        if(!open) {
            position = 0;
        }

        if(position < more_lines_below) {
            more_lines_below = position;
        }

        int margin = x_margin;
        int y = 480 - this_line_height;
        std::size_t i = position;
        auto resolution = get_resolution();
        int width = (widescreen_fix_enabled() ? (static_cast<int>(resolution.width) * 480 + 240) / resolution.height : 640) - margin * 2;
        auto now = SteadyClock::now();

        // If we're on the bottom, we can clear this
        if(position == 0) {
            more_lines_below = 0;
        }

        // Set up font data
        auto &font_data = get_current_font_data();
        for(int i = 0; i < 5; i++) {
            font_data.tabs[i] = width / 4 * i;
        }

        // Show the text input?
        if(open) {
            // Get the prompt color
            static ColorARGB *console_color = nullptr;
            if(!console_color) {
                if(get_chimera().feature_present("client_console_prompt_color_demo")) {
                    console_color = reinterpret_cast<ColorARGB *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("console_prompt_color_demo_sig").data() + 2) - 4);
                }
                else {
                    console_color = *reinterpret_cast<ColorARGB **>(get_chimera().get_signature("console_prompt_color_sig").data() + 1);
                }
            }
            console_color->alpha = 0.0F;

            // Copy the console color
            ColorARGB color = *console_color;
            color.alpha = 1.0F;

            // Note that we have things below
            int right_side = -margin;
            if(more_lines_below) {
                char more_lines_below_text[256];
                std::snprintf(more_lines_below_text, sizeof(more_lines_below_text), "(+%zu line%s)", more_lines_below, more_lines_below == 1 ? "" : "s");
                int more_lines_below_width = text_pixel_length(more_lines_below_text, font);
                right_side += more_lines_below_width;
                apply_text(more_lines_below_text, width - right_side, y, more_lines_below_width, height, color, font, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_TOP_LEFT);
            }

            // Get the console text
            const auto *console_text = get_console_text();
            const auto *prompt_pre = console_text - 32;

            // Show the prompt prefix, aka "halo( "
            auto prefix_x = margin + text_pixel_length(prompt_pre, font);
            apply_text(prompt_pre, margin, y, width, height, color, font, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_TOP_LEFT);

            // Show the remaining text
            std::uint8_t cursor = static_cast<std::uint8_t>(console_text[0x106]);
            apply_text(console_text, prefix_x, y, width - (prefix_x + right_side), height, color, font, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_TOP_LEFT);

            // Blink a cursor every second?
            if(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 500 % 2) {
                int cursor_x = 0;

                // Are we at the end? If so, we don't need to make a substring
                if(console_text[cursor] == 0) {
                    cursor_x = text_pixel_length(console_text, font);
                }
                // Otherwise, a meme we go
                else {
                    cursor_x = text_pixel_length(std::string(console_text, console_text + cursor).c_str(), font);
                }
                apply_text("_", prefix_x + cursor_x, y + height + (line_height - height), width, height, color, font, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_TOP_LEFT);
            }
        }

        y -= this_line_height * 1.2;

        // Is the button held? (prevent scrolling too fast)
        static bool button_held = false;

        // Are we pressing things?
        auto &keys = get_keyboard_keys();
        if(keys.page_up || keys.page_down) {
            int max_lines_shown = y / this_line_height;
            int half_page = max_lines_shown / 2;

            if(!button_held) {
                button_held = true;
                if(keys.page_up) {
                    position += half_page;
                    if(position >= custom_lines.size()) {
                        position = custom_lines.size() - 1;
                    }
                }
                else {
                    if(position < static_cast<std::size_t>(half_page)) {
                        position = 0;
                    }
                    else {
                        position -= half_page;
                    }
                }
            }
        }
        else {
            button_held = false;
        }

        // Show the lines
        while(y > 0 && i < custom_lines.size()) {
            auto &line = custom_lines[custom_lines.size() - (i + 1)];
            auto color_copy = line.color;

            // Renew the lifespan of still-alive lines if open
            auto time_since = std::chrono::duration_cast<std::chrono::microseconds>(now - line.created).count();
            if(open) {
                if(time_since < total_lifetime * MICROSECONDS_PER_SEC) {
                    line.created = now;
                    time_since = 0;
                }
            }

            // If we aren't open or scrollback is disabled, hide lines that aren't alive anymore
            if(!open || !use_scrollback) {
                if(time_since > total_lifetime * MICROSECONDS_PER_SEC) {
                    break;
                }
                if(time_since > fade_start * MICROSECONDS_PER_SEC) {
                    double scale = 1.0 - (time_since - fade_start * MICROSECONDS_PER_SEC) / (fade_time * MICROSECONDS_PER_SEC);
                    color_copy.alpha *= scale;
                }
            }

            apply_text(line.text, margin, y, width, height, color_copy, font, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_TOP_LEFT);
            i++;
            y -= this_line_height;
        }

        // Clean up if not open
        if(!open && use_scrollback) {
            while(custom_lines.size() > max_lines_soft) {
                if(std::chrono::duration_cast<std::chrono::microseconds>(now - custom_lines.front().created).count() > total_lifetime * MICROSECONDS_PER_SEC) {
                    custom_lines.pop_front();
                }
                else {
                    break;
                }
            }
        }
    }

    static void do_cls() noexcept {
        // If we ran cls in the console, actually clear it
        if(get_console_open()) {
            position = 0;
            custom_lines.clear();
        }
        // Otherwise, make it look like we cleared it but if we have scrollback, we can see those messages again
        else {
            auto invalid_time = SteadyClock::now() - std::chrono::microseconds(static_cast<int>((fade_start + fade_time) * 1000000));
            for(auto &i : custom_lines) {
                i.created =invalid_time;
            }
        }
    }

    void setup_custom_console() noexcept {
        // Intercept console text printed
        static Hook out_hook;
        const void *original_fn;
        auto &chimera = get_chimera();
        if(chimera.feature_present("client_demo")) {
            write_function_override(chimera.get_signature("console_out_copy_demo_sig").data(), out_hook, reinterpret_cast<const void *>(override_console_output_edi_asm), &original_fn);
        }
        else {
            write_function_override(chimera.get_signature("console_out_copy_sig").data(), out_hook, reinterpret_cast<const void *>(override_console_output_eax_asm), &original_fn);
        }

        static Hook cls_hook;
        write_jmp_call(chimera.get_signature("console_cls_sig").data(), cls_hook, reinterpret_cast<const void *>(do_cls));

        auto *ini = chimera.get_ini();
        max_lines = ini->get_value_size("custom_console.buffer_size").value_or(10000);
        max_lines_soft = ini->get_value_size("custom_console.buffer_size_soft").value_or(256);
        use_scrollback = ini->get_value_bool("custom_console.enable_scrollback").value_or(false);
        line_height = ini->get_value_float("custom_console.line_height").value_or(1.1);
        fade_time = ini->get_value_float("custom_console.fade_time").value_or(0.75);
        fade_start = ini->get_value_float("custom_console.fade_start").value_or(4.0);
        x_margin = ini->get_value_long("custom_console.x_margin").value_or(10);

        total_lifetime = fade_time + fade_start;

        add_preframe_event(on_console_frame);
    }

    extern "C" void on_console_output(char *text, const ColorARGB &color) {
        // Erase all new lines until the buffer size is small enough to hold the new line
        while(custom_lines.size() > max_lines + 1) {
            custom_lines.pop_front();
            if(position > 1) {
                position--;
            }
        }

        custom_lines.emplace_back(Line { std::string(text), SteadyClock::now(), color });
        text[0] = 0;

        // Add 1 so we don't lose our position
        if(position > 0) {
            position++;
            more_lines_below ++; // tell the user more lines have been printed
        }
    }
}
