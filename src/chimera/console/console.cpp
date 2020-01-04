#include "../localization/localization.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../command/command.hpp"
#include "../event/command.hpp"
#include "../event/tick.hpp"
#include "../output/output.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/multiplayer.hpp"
#include "console.hpp"

#include <fstream>

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


    static CommandEntry ***entries;
    static std::uint32_t *entry_count;
    static CommandEntry **old_entries;
    static std::uint32_t old_entry_count;

    static std::vector<CommandEntry *> new_entries_list;
    static std::vector<std::unique_ptr<CommandEntry>> new_entries_added;

    void script_command_dump_command(int, const char **) noexcept {
        std::size_t command_entry_count = *entry_count;
        auto *command_entries = *entries;

        char path[MAX_PATH];
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
            case GameEngine::GAME_ENGINE_CUSTOM_EDITION:
                sig_to_use = "command_list_custom_edition_sig";
                break;
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
                else {
                    console_text[0] = 0;
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
}
