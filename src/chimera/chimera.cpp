// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <optional>
#include "annoyance/novideo.hpp"
#include "annoyance/tab_out_video.hpp"
#include "bookmark/bookmark.hpp"
#include "custom_chat/custom_chat.hpp"
#include "config/config.hpp"
#include "console/console.hpp"
#include "localization/localization.hpp"
#include "halo_data/script.hpp"
#include "event/frame.hpp"
#include "halo_data/path.hpp"
#include "halo_data/hud_fonts.hpp"
#include "lua/scripting.hpp"
#include "output/draw_text.hpp"
#include "output/output.hpp"
#include "signature/hook.hpp"
#include "signature/signature.hpp"
#include "chimera.hpp"
#include "annoyance/auto_get_list.hpp"
#include "annoyance/drm.hpp"
#include "annoyance/keystone.hpp"
#include "annoyance/multi_instance.hpp"
#include "annoyance/registry.hpp"
#include "annoyance/update.hpp"
#include "annoyance/watson.hpp"
#include "master_server/master_server.hpp"
#include "map_loading/map_loading.hpp"
#include "map_loading/fast_load.hpp"
#include "fix/auto_center.hpp"
#include "fix/abolish_safe_mode.hpp"
#include "fix/aim_assist.hpp"
#include "fix/bullshit_server_data.hpp"
#include "fix/invalid_command_crash.hpp"
#include "fix/death_reset_time.hpp"
#include "fix/descope_fix.hpp"
#include "fix/extend_limits.hpp"
#include "fix/flashlight_fix.hpp"
#include "fix/extended_description_fix.hpp"
#include "fix/name_fade.hpp"
#include "fix/camera_shake_fix.hpp"
#include "fix/checkpoint_fix.hpp"
#include "fix/scoreboard_fade_fix.hpp"
#include "fix/floor_decal_memery.hpp"
#include "fix/fov_fix.hpp"
#include "fix/fp_reverb.hpp"
#include "fix/force_crash.hpp"
#include "fix/leak_descriptors.hpp"
#include "fix/nav_numbers.hpp"
#include "fix/timer_offset.hpp"
#include "fix/sane_defaults.hpp"
#include "fix/vehicle_team_desync.hpp"
#include "fix/uncompressed_sound_fix.hpp"
#include "fix/video_mode.hpp"
#include "fix/model_detail.hpp"
#include "fix/blue_32bit_color_fix.hpp"
#include "fix/contrail_fix.hpp"
#include "fix/interpolate/interpolate.hpp"
#include "fix/sun_fix.hpp"
#include "fix/flashlight_fix.hpp"
#include "fix/motion_sensor_fix.hpp"
#include "fix/inverted_flag.hpp"
#include "halo_data/object.hpp"
#include "event/tick.hpp"
#include "event/map_load.hpp"
#include "fix/custom_map_lobby_fix.hpp"
#include "fix/weapon_swap_ticks.hpp"
#include "halo_data/game_engine.hpp"
#include "halo_data/main_menu_music.hpp"
#include "halo_data/multiplayer.hpp"
#include "miscellaneous/controller.hpp"
#include "halo_data/port.hpp"
#include "command/hotkey.hpp"
#include "config/ini.hpp"
#include "annoyance/exception_dialog.hpp"
#include "output/error_box.hpp"
#include "fix/biped_ui_spawn.hpp"

namespace Chimera {
    static Chimera *chimera;
    static void initial_tick();
    static void set_up_delayed_init();
    static void april_fools() noexcept;

    Chimera::Chimera() : p_signatures(find_all_signatures()) {
        chimera = this;

        // Set the locale to match the system
        std::setlocale(LC_CTYPE, "");

        // If we *can* load Chimera, then do it
        if(find_signatures()) {
            const char *build_string = *reinterpret_cast<const char **>(this->get_signature("build_string_sig").data() + 1);
            static const char *expected_version = "01.00.10.0621";
            if(game_engine() != GAME_ENGINE_DEMO && std::strcmp(build_string, expected_version) != 0) {
                char error[256] = {};
                std::snprintf(error, sizeof(error), "Chimera does not support %s. Please use %s.", build_string, expected_version);
                show_error_box("Error", error);
                this->p_signatures.clear();
                return;
            }

            this->get_all_commands();
            initialize_console_hook();

            // Make this thing less annoying
            reduce_drm();

            // Goodbye, Watson
            remove_watson();

            // Load the ini
            chimera->reload_ini();

            // Set the path
            const char *path = chimera->get_ini()->get_value("halo.path");
            if(path) {
                static std::string new_path = path;
                if(new_path.size() >= MAX_PATH) {
                    show_error_box("Error", "Path is too long");
                    std::exit(1);
                }
                overwrite(chimera->get_signature("write_path_sig").data() + 2, new_path.data());
            }

            // Enable fast loading
            initialize_fast_load();

            // Set up map loading
            set_up_map_loading();

            // ...why Gearbox?
            set_up_timer_offset_fix();

            // Fix this
            set_up_fix_leaking_descriptors();

            // Lol
            set_up_invalid_command_crash_fix();

            // The end of an era
            set_up_remove_exception_dialog();

            if(this->feature_present("client")) {
                // Fix the camo by default
                extern bool camo_fix_command(int argc, const char **argv);
                const char *value_true = "true";
                camo_fix_command(1, &value_true);
                add_preframe_event(initial_tick);

                // Fix some more bullshit
                set_up_floor_decals_fix();

                add_map_load_event(april_fools);

                // Set up this hook
                set_up_rcon_message_hook();

                // Fix camera shake
                set_up_camera_shake_fix();

                // Fix the checkpoints
                set_up_checkpoint_fix();

                // Fix more bullshit that Gearbox could've fixed but didn't
                set_up_blue_32bit_color_fix();
                set_up_contrail_fix();
                set_up_sun_fix();
                set_up_motion_sensor_fix();
                set_up_flashlight_fix();
                set_up_inverted_flag_fix();

                // No more updates
                enable_block_update_check();

                // Make the game use max settings as default because it's not 2003 anymore
                set_up_sane_defaults();

                // Prevent some annoying registry checks that just make the game slower
                remove_registry_checks();

                // Disable this crashy piece of shit that some asshole at Microsoft thought was a good idea to put in a game
                remove_keystone();

                // Fix vehicle desyncing with mtv
                set_up_vehicle_team_desync_fix();

                // Fix clans putting invisible bullshit in their server names to put them at the top of the list
                set_up_bullshit_server_data_fix();

                // Well, why not?
                set_up_auto_get_list();

                // Do this!
                if(chimera->get_ini()->get_value_bool("halo.background_playback").value_or(false)) {
                    enable_tab_out_video();
                }

                // And this!
                if(chimera->get_ini()->get_value_bool("halo.multiple_instances").value_or(false)) {
                    enable_multiple_instance();
                }

                set_cd_hash();

                // Speed up getting connections from the master server
                set_master_server_connection_threads(50);

                // Fix maps that go above some of Halo's limits
                set_up_extend_limits();

                // Fix this broken stuff
                set_up_auto_center_fix();

                // No interpolation in a 2003 PC game? Seriously, Gearbox?
                set_up_interpolation();

                // Set video mode
                set_up_video_mode();

                // Fix the damn FOV
                set_up_fov_fix();

                // And this should get fixed, too. Holy shit.
                set_up_descope_fix();

                // Fix this massive gameplay issue
                set_up_weapon_swap_ticks_fix();

                // Why is this broken?
                if(chimera->feature_present("client_widescreen_custom_edition")) {
                    set_up_aim_assist_fix();
                }

                // Why would Halo PC even have this check? lol
                if(game_engine() == GameEngine::GAME_ENGINE_RETAIL && chimera->feature_present("client_retail_custom_map_lobby")) {
                    set_up_custom_map_lobby_fix();
                }

                // Fix the master server
                set_up_master_server();

                set_up_model_detail_fix();

                // This could be useful
                set_up_server_history();

                // Someone might want this set of course
                set_up_scoreboard_font();
                set_up_name_font();

                // lol
                set_up_nav_numbers_fix();
                set_up_name_fade_fix();
                set_up_scoreboard_fade_fix();

                // More lol
                set_up_extended_description_fix();

                // wtf
                set_up_force_crash_fix();

                // Fuck this
                set_up_abolish_safe_mode();

                // Remove the video on start/end
                if(!chimera->get_ini()->get_value_bool("halo.intro_videos").value_or(false)) {
                    enable_novideo();
                }

                // Maybe disable main menu music?
                block_main_menu_music_if_needed();

                // Do this
                set_ports();

                // Chu
                set_up_hotkeys();

                // Memes
                set_up_uncompressed_sound_fix();

                // RAICHU
                set_up_controller();

                // Fix bipeds spawning on UI, oh the horror
                set_up_fix_biped_ui_spawn();
            }
            else {
                enable_output(true);
            }

            // Make it so number one
            set_up_delayed_init();
        }
    }

    bool Chimera::feature_present(const char *feature) {
        // Look for the super duper feature first
        if(std::strcmp(feature, "client") == 0 || std::strcmp(feature, "server") == 0) {
            if(!feature_present("core")) {
                return false;
            }
        }

        // Look for the super feature next
        if(std::strncmp(feature, "client_", 7) == 0) {
            if(!feature_present("client")) {
                return false;
            }
        }
        if(std::strncmp(feature, "server_", 7) == 0) {
            if(!feature_present("server")) {
                return false;
            }
        }

        // Also format feature into feature_<retail / custom_edition / demo>
        char feature_arch[256];
        const char *name_str = nullptr;
        bool full_version = false;
        switch(game_engine()) {
            case GameEngine::GAME_ENGINE_CUSTOM_EDITION:
                name_str = "custom_edition";
                full_version = true;
                break;
            case GameEngine::GAME_ENGINE_RETAIL:
                name_str = "retail";
                full_version = true;
                break;
            case GameEngine::GAME_ENGINE_DEMO:
                name_str = "demo";
                break;
        }
        std::snprintf(feature_arch, sizeof(feature_arch), "%s_%s", feature, name_str);

        char feature_arch_2[256];
        std::snprintf(feature_arch_2, sizeof(feature_arch_2), "%s_%s", feature, full_version ? "full" : "(null)");

        // Check for it now
        bool feature_found = false;
        for(auto &signature : this->p_signatures) {
            if(std::strcmp(signature.feature(), feature) == 0 || std::strcmp(signature.feature(), feature_arch) == 0 || std::strcmp(signature.feature(), feature_arch_2) == 0) {
                feature_found = true;
                if(signature.data() == 0) {
                    return false;
                }
            }
        }
        return feature_found;
    }

    Signature &Chimera::get_signature(const char *signature) {
        // Find the signature
        for(auto &sig : this->p_signatures) {
            if(std::strcmp(sig.name(), signature) == 0) {
                return sig;
            }
        }

        // We can't feasibly continue from this without causing undefined behavior. Abort the process after showing an error message.
        char error[256];
        std::snprintf(error, sizeof(error), "CHIMERA ERROR: Signature %s is invalid.\n\nNote: This is a bug.\n", signature);
        show_error_box("Chimera error", error);
        std::exit(135);
    }

    std::vector<const char *> Chimera::missing_signatures_for_feature(const char *feature) {
        std::vector<const char *> signatures_missing;
        for(auto &signature : this->p_signatures) {
            if(std::strcmp(signature.feature(), feature) == 0 && signature.data() == 0) {
                signatures_missing.push_back(signature.name());
            }
        }
        return signatures_missing;
    }

    CommandResult Chimera::execute_command(const char *command, const Command **found_command, bool saves) {
        // Try to parse it
        auto arguments = split_arguments(command);

        // Check if there actually was something given
        if(arguments.size() != 0) {
            // Get the command name and lowercase it
            std::string command_name = arguments[0];
            for(char &c : command_name) {
                c = std::tolower(c);
            }

            // Remove the command name from the arguments
            arguments.erase(arguments.begin());

            // Find and execute the command
            for(auto &cmd : this->p_commands) {
                if(std::strcmp(command_name.data(), cmd.name()) == 0) {
                    if(found_command) {
                        *found_command = &cmd;
                    }
                    extern const char *output_prefix;
                    auto *old_prefix = output_prefix;
                    if(std::strcmp(cmd.name(), "chimera") == 0) {
                        output_prefix = nullptr;
                    }
                    else {
                        output_prefix = cmd.name();
                    }
                    auto result = cmd.call(arguments);
                    output_prefix = old_prefix;

                    if(saves && this->p_config.get() && result == CommandResult::COMMAND_RESULT_SUCCESS && cmd.autosave() && arguments.size() > 0) {
                        this->p_config->set_settings_for_command(command_name.data(), arguments);
                    }
                    return result;
                }
            }
        }
        return CommandResult::COMMAND_RESULT_FAILED_ERROR_NOT_FOUND;
    }

    Language Chimera::get_language() const noexcept {
        return this->p_language;
    }

    void Chimera::set_language(Language language) noexcept {
        if(this->p_language != language) {
            this->p_language = language;
            this->get_all_commands();
        }
    }

    Chimera &get_chimera() {
        return *chimera;
    }

    extern "C" void instantiate_chimera() {
        chimera = new Chimera();
    }

    extern "C" void destroy_chimera() {
        delete chimera;
    }

    #define SECONDARY_FEATURE (halo_type() == 2 ? "server" : "client")

    extern "C" int find_signatures() {
        return chimera->feature_present("client") || chimera->feature_present("server");
    }

    extern "C" void print_signature_errors() {
        // Hold errors
        static char error_buffer[65536];
        std::size_t error_buffer_offset = 0;
        #define APPEND_SPRINTF(...) error_buffer_offset += std::snprintf(error_buffer + error_buffer_offset, sizeof(error_buffer) - error_buffer_offset, __VA_ARGS__)

        APPEND_SPRINTF("Could not load Chimera.\n\n");
        bool engine_type_missing = false;
        bool is_server = chimera->get_signature("map_server_path_1_sig").data() != nullptr;

        auto list_missing_sigs_for_feature = [&error_buffer_offset, &engine_type_missing, &is_server](const char *feature) {
            // If we're the server, don't show non-server signatures and vice versa
            bool client_signature = std::strncmp(feature, "client", 6) == 0;
            bool server_signature = std::strncmp(feature, "server", 6) == 0;
            if((client_signature && is_server) || (server_signature && !is_server)) {
                return;
            }

            auto missing_sigs = chimera->missing_signatures_for_feature(feature);
            if(missing_sigs.size() != 0) {
                APPEND_SPRINTF("Missing signatures from %s:\n", feature);
                for(auto &sig : missing_sigs) {
                    APPEND_SPRINTF("    - %s\n", sig);
                    if(std::strcmp(sig, "game_engine_sig") == 0) {
                        engine_type_missing = true;
                    }
                }
                APPEND_SPRINTF("\n");
            }
        };

        list_missing_sigs_for_feature("core");
        list_missing_sigs_for_feature("client");
        list_missing_sigs_for_feature("server");

        if(!engine_type_missing) {
            switch(game_engine()) {
                case GameEngine::GAME_ENGINE_CUSTOM_EDITION:
                    list_missing_sigs_for_feature("core_custom_edition");
                    list_missing_sigs_for_feature("client_custom_edition");
                    list_missing_sigs_for_feature("server_custom_edition");
                    list_missing_sigs_for_feature("core_full");
                    list_missing_sigs_for_feature("client_full");
                    list_missing_sigs_for_feature("server_full");
                    break;
                case GameEngine::GAME_ENGINE_RETAIL:
                    list_missing_sigs_for_feature("core_retail");
                    list_missing_sigs_for_feature("client_retail");
                    list_missing_sigs_for_feature("server_retail");
                    list_missing_sigs_for_feature("core_full");
                    list_missing_sigs_for_feature("client_full");
                    list_missing_sigs_for_feature("server_full");
                    break;
                case GameEngine::GAME_ENGINE_DEMO:
                    list_missing_sigs_for_feature("core_demo");
                    list_missing_sigs_for_feature("client_demo");
                    break;
            }
        }
        show_error_box("Error", error_buffer, true, !is_server);
    }

    int halo_type() {
        return chimera->feature_present("client") ? 1 : chimera->feature_present("server") ? 2 : 0;
    }

    extern "C" const std::byte *address_for_signature(const char *signature) {
        return chimera->get_signature(signature).data();
    }

    extern "C" void address_for_signature_if_null(const char *signature, std::byte **address) {
        if(*address == nullptr) {
            *address = chimera->get_signature(signature).data();
        }
    }

    const std::filesystem::path Chimera::get_path() noexcept {
        if(this->p_path.empty()) {
            this->p_path = std::filesystem::path(halo_path()) / "chimera";
            std::filesystem::create_directory(this->p_path);
            std::filesystem::create_directory(this->p_path / "tmp");
        }
        return this->p_path;
    }

    const std::filesystem::path Chimera::get_download_map_path() noexcept {
        if (this->p_download_map_path.empty()){
            const char *path = this->get_ini()->get_value("halo.download_map_path");
            if (path){
                this->p_download_map_path = std::filesystem::path(path);
            }
            else {
                this->p_download_map_path = this->get_path() / "maps";
            }
            std::filesystem::create_directory(this->p_download_map_path);
        }
        return this->p_download_map_path;
    }

    const std::filesystem::path Chimera::get_map_path() noexcept {
        if (this->p_map_path.empty()){
            const char *path = this->get_ini()->get_value("halo.map_path");
            if (path){
                this->p_map_path = std::filesystem::path(path);
            }
            else {
                this->p_map_path = std::filesystem::path("maps");
            }
            std::filesystem::create_directory(this->p_map_path);
        }
        return this->p_map_path;
    }

    void Chimera::reload_config() {
        this->p_config = std::make_unique<Config>(this->get_path() / "preferences.txt");
        this->p_config->load();
    }

    void Chimera::reload_ini() {
        this->p_ini = std::make_unique<Ini>("chimera.ini");
    }

    const Ini *Chimera::get_ini() const noexcept {
        return this->p_ini.get();
    }

    void initial_tick() {
        remove_preframe_event(initial_tick);
        if(chimera->feature_present("client")) {
            // Set up the text hook?
            setup_text_hook();

            // Set default settings
            if(chimera->get_ini()->get_value_bool("halo.optimal_defaults").value_or(false)) {
                chimera->execute_command("chimera_block_mouse_acceleration true");
                chimera->execute_command("chimera_aim_assist true");
                chimera->execute_command("chimera_diagonals 0.75");
                chimera->execute_command("chimera_block_loading_screen true");
                chimera->execute_command("chimera_fov auto");
                chimera->execute_command("chimera_fov_cinematic auto");
                chimera->execute_command("chimera_fp_reverb true");
                chimera->execute_command("chimera_throttle_fps 300");
                chimera->execute_command("chimera_uncap_cinematic true");
                if(chimera->feature_present("client_af")) {
                    chimera->execute_command("chimera_af true");
                }
            }

            // Also set these fixes
            char buffer[256];
            auto &commands = chimera->get_commands();
            if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
                for(std::size_t i = 0; i < commands.size(); i++) {
                    auto &command = commands[i];
                    if(std::strcmp(command.category(), "chimera_category_fix") == 0) {
                        std::snprintf(buffer, sizeof(buffer), "%s true", command.name());
                        chimera->execute_command(buffer);
                        commands.erase(commands.begin() + i);
                        i--;
                    }
                }
            }
            else {
                for(auto &i : commands) {
                    if(std::strcmp(i.category(), "chimera_category_fix") == 0) {
                        std::snprintf(buffer, sizeof(buffer), "%s true", i.name());
                        chimera->execute_command(buffer);
                    }
                }
            }

            // Set console enabled
            if(chimera->get_ini()->get_value_bool("halo.console").value_or(false)) {
                set_console_enabled(true);
            }

            // Load the custom chat
            initialize_custom_chat();

            // If we're using a custom console, set that up
            if(chimera->get_ini()->get_value_bool("custom_console.enabled").value_or(false)) {
                setup_custom_console();
            }
            // Otherwise, merely fix console fade
            else {
                // Fix console fade
                setup_console_fade_fix();
            }

            // Fix the death reset time
            setup_death_reset_time_fix();

            chimera->reload_config();
        }

        // Set up Lua scripting
        setup_lua_scripting();

        enable_output(true);
    }

    const std::vector<Command> &Chimera::get_commands() const noexcept {
        return this->p_commands;
    }

    std::vector<Command> &Chimera::get_commands() noexcept {
        return this->p_commands;
    }

    static void execute_init() {
        // Don't re-execute this
        remove_frame_event(execute_init);
        bool should_error_if_not_found = false;
        bool is_server = get_chimera().feature_present("server");

        // First see if we set anything here
        const char *init = get_chimera().get_ini()->get_value("halo.exec");
        if(init) {
            should_error_if_not_found = true;
        }
        else {
            init = "init.txt";
        }

        // Next, check the command-line args
        auto *cmd_line = GetCommandLineA();
        char *first_letter = cmd_line;
        std::string last_param;
        std::optional<std::string> init_maybe;
        bool letter_digested = false;
        bool inside_quote = false;
        for(char *c = cmd_line; cmd_line == c || c[-1]; c++) {
            // Invert if we're inside quotes
            if(*c == '"') {
                inside_quote = !inside_quote;
            }

            if((*c == 0 || (*c == ' ' && !inside_quote)) && letter_digested) {
                std::string this_param = std::string(first_letter, c - first_letter);
                if(last_param == "-exec") {
                    init_maybe = this_param;
                    break;
                }
                first_letter = c + 1;
                last_param = this_param;
                letter_digested = false;
            }
            else if(*c != ' ' && !letter_digested) {
                letter_digested = true;
            }
            else if(*c == ' ' && !letter_digested) {
                first_letter = c + 1;
            }
        }

        // Remove quotes from it
        char final_init_maybe[MAX_PATH] = {};
        if(init_maybe.has_value()) {
            auto init_maybe_len = init_maybe->size();
            auto *init_maybe_cstr = init_maybe->c_str();
            for(std::size_t i = 0, k = 0; i < (sizeof(final_init_maybe) - 1) && k < init_maybe_len; i++, k++) {
                while(init_maybe_cstr[k] == '"') {
                    k++;
                }
                final_init_maybe[i] = init_maybe_cstr[k];
            }
            init = final_init_maybe;
            should_error_if_not_found = true;
        }

        // Do it!
        if(!is_server) {
            get_chimera().get_config().set_saving(false);
        }
        std::fstream file_to_open(init, std::ios_base::in);
        if(file_to_open.is_open()) {
            std::string line;
            while(std::getline(file_to_open, line)) {
                if(std::strncmp(line.c_str(), "chimera", strlen("chimera")) == 0) {
                    const Command *found_command;
                    switch(get_chimera().execute_command(line.c_str(), &found_command)) {
                        case COMMAND_RESULT_SUCCESS:
                        case COMMAND_RESULT_FAILED_ERROR:
                            break;
                        case COMMAND_RESULT_FAILED_FEATURE_NOT_AVAILABLE:
                            console_error(localize("chimera_error_command_unavailable"), found_command->name(), found_command->feature());
                            break;
                        case COMMAND_RESULT_FAILED_ERROR_NOT_FOUND:
                            console_error(localize("chimera_error_command_not_found"));
                            break;
                        case COMMAND_RESULT_FAILED_NOT_ENOUGH_ARGUMENTS:
                            console_error(localize("chimera_error_not_enough_arguments"), found_command->name(), found_command->min_args());
                            break;
                        case COMMAND_RESULT_FAILED_TOO_MANY_ARGUMENTS:
                            console_error(localize("chimera_error_too_many_arguments"), found_command->name(), found_command->max_args());
                            break;
                    }
                }
                else {
                    execute_script(line.c_str());
                }
            }
        }
        else if(should_error_if_not_found) {
            console_error(localize("chimera_error_failed_to_open_init"), init);
        }
        if(!is_server) {
            get_chimera().get_config().set_saving(true);
        }
    }

    static void set_up_delayed_init() {
        auto &chimera = get_chimera();
        overwrite(chimera.get_signature("exec_init_sig").data(), static_cast<std::uint8_t>(0xC3));
        add_frame_event(execute_init);
    }

    // Have fun
    static void cartridge_tilt() noexcept {
        auto &table = ObjectTable::get_object_table();

        if((get_tick_count() / 16) % 15) {
            return;
        }

        static std::size_t offset = 0;
        #define RND (((((offset++) ^ 0x123456) * 7891) % 256) / 256.0 - 0.5 * 2.0)

        for(std::size_t i = 0; i < table.current_size; i++) {
            auto *object = table.get_dynamic_object(i);
            if(object) {
                auto *nodes = object->nodes();

                std::size_t node_count;

                if(object->type != ObjectType::OBJECT_TYPE_PROJECTILE) {
                    auto *tag_data = get_tag(object->tag_id)->data;
                    auto *model_tag = get_tag(*reinterpret_cast<TagID *>(tag_data + 0x28 + 0xC));
                    if(!model_tag) {
                        continue;
                    }
                    node_count = *reinterpret_cast<std::uint32_t *>(model_tag->data + 0xB8);
                }
                else {
                    node_count = 1;
                }

                if(nodes) {
                    for(std::size_t n = 0; n < node_count; n++) {
                        auto *node = nodes + n;
                        node->position.x += RND;
                        node->position.y += RND;
                        node->position.z += RND;
                        node->scale += RND;
                        node->rotation.v[0].x += RND;
                        node->rotation.v[0].y -= RND;
                        node->rotation.v[0].z += RND;
                        node->rotation.v[1].x -= RND;
                        node->rotation.v[1].y += RND;
                        node->rotation.v[1].z -= RND;
                        node->rotation.v[2].x -= RND;
                        node->rotation.v[2].y += RND;
                        node->rotation.v[2].z -= RND;
                    }
                }
            }
        }
    }

    // Every 1st of April, the Master Chief gets down
    static void april_fools() noexcept {
        remove_preframe_event(cartridge_tilt);
        SYSTEMTIME time = {};
        GetSystemTime(&time);
        if(time.wMonth == 4 && time.wDay == 1) {
            const char *map_name;
            if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
                auto &demo_map_header = get_demo_map_header();
                map_name = demo_map_header.name;
            }
            else {
                auto &map_header = get_map_header();
                map_name = map_header.name;
            }

            // Only cartridge tilt on ui.map
            if(std::strcmp(map_name, "ui") == 0) {
                add_preframe_event(cartridge_tilt, EVENT_PRIORITY_AFTER);
            }
        }
    }
}
