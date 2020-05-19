#include <windows.h>
#include <iostream>
#include <cstring>
#include <cmath>
#include "annoyance/novideo.hpp"
#include "annoyance/tab_out_video.hpp"
#include "bookmark/bookmark.hpp"
#include "custom_chat/custom_chat.hpp"
#include "config/config.hpp"
#include "console/console.hpp"
#include "event/frame.hpp"
#include "halo_data/path.hpp"
#include "halo_data/hud_fonts.hpp"
#include "output/draw_text.hpp"
#include "output/output.hpp"
#include "signature/hook.hpp"
#include "signature/signature.hpp"
#include "chimera.hpp"
#include "annoyance/drm.hpp"
#include "annoyance/keystone.hpp"
#include "annoyance/multi_instance.hpp"
#include "annoyance/registry.hpp"
#include "master_server/master_server.hpp"
#include "map_loading/map_loading.hpp"
#include "fast_load/fast_load.hpp"
#include "fix/auto_center.hpp"
#include "fix/abolish_safe_mode.hpp"
#include "fix/blue_32bit_color_fix.hpp"
#include "fix/aim_assist.hpp"
#include "fix/death_reset_time.hpp"
#include "fix/descope_fix.hpp"
#include "fix/extend_limits.hpp"
#include "fix/inverted_flag.hpp"
#include "fix/fov_fix.hpp"
#include "fix/force_crash.hpp"
#include "fix/leak_descriptors.hpp"
#include "fix/motion_sensor_fix.hpp"
#include "fix/nav_numbers.hpp"
#include "fix/sun_fix.hpp"
#include "fix/contrail_fix.hpp"
#include "fix/vehicle_team_desync.hpp"
#include "fix/uncompressed_sound_fix.hpp"
#include "fix/video_mode.hpp"
#include "fix/model_detail.hpp"
#include "fix/custom_map_lobby_fix.hpp"
#include "fix/weapon_swap_ticks.hpp"
#include "halo_data/game_engine.hpp"
#include "halo_data/main_menu_music.hpp"
#include "halo_data/port.hpp"
#include "command/hotkey.hpp"
#include "config/ini.hpp"

namespace Chimera {
    static Chimera *chimera;
    static void initial_tick();

    Chimera::Chimera() : p_signatures(find_all_signatures()) {
        chimera = this;

        // If we *can* load Chimera, then do it
        if(find_signatures()) {
            const char *build_string = *reinterpret_cast<const char **>(this->get_signature("build_string_sig").data() + 1);
            static const char *expected_version = "01.00.10.0621";
            if(game_engine() != GAME_ENGINE_DEMO && std::strcmp(build_string, expected_version) != 0) {
                char error[256] = {};
                std::snprintf(error, sizeof(error), "Chimera does not support %s. Please update your game to %s.", build_string, expected_version);
                MessageBox(0, error, "Error", 0);
                this->p_signatures.clear();
                return;
            }

            this->get_all_commands();
            initialize_console_hook();

            // Make this thing less annoying
            reduce_drm();

            // Load the ini
            chimera->reload_ini();

            // Set the path
            const char *path = chimera->get_ini()->get_value("halo.path");
            if(path) {
                static std::string new_path = path;
                if(new_path.size() >= MAX_PATH) {
                    MessageBox(nullptr, "Path is too long", "Error", 0);
                    std::exit(1);
                }
                overwrite(chimera->get_signature("write_path_sig").data() + 2, new_path.data());
            }

            // Enable fast loading
            initialize_fast_load();

            // Set up map loading
            set_up_map_loading();

            // Fix this
            set_up_fix_leaking_descriptors();

            if(this->feature_present("client")) {
                // Fix the camo by default
                extern bool camo_fix_command(int argc, const char **argv);
                const char *value_true = "true";
                camo_fix_command(1, &value_true);
                add_preframe_event(initial_tick);

                // Set up this hook
                set_up_rcon_message_hook();

                // Prevent some annoying registry checks that just make the game slower
                remove_registry_checks();

                // Disable this crashy piece of shit that some asshole at Microsoft thought was a good idea to put in a game
                remove_keystone();

                // Fix vehicle desyncing with mtv
                set_up_vehicle_team_desync_fix();

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

                // Set video mode
                set_up_video_mode();

                // Fix the damn FOV
                set_up_fov_fix();

                // And this should get fixed, too. Holy shit.
                set_up_descope_fix();

                // More broken stuff. More broken fixes.
                set_up_inverted_flag_fix();

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

                // Fix some demo stuff
                if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
                    set_up_demo_master_server();
                }

                // Last I checked, not even MCC has this fixed lol.
                set_up_sun_fix();

                // Or this?
                set_up_model_detail_fix();

                // This could be useful
                set_up_server_history();

                // Someone might want this set of course
                set_up_scoreboard_font();
                set_up_name_font();

                // lol
                set_up_nav_numbers_fix();
                set_up_contrail_fix();
                set_up_motion_sensor_fix();

                // More lol
                set_up_blue_32bit_color_fix();

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
            }
            else {
                enable_output(true);
            }
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
        std::snprintf(error, sizeof(error), "CHIMERA ERROR: Signature %s is invalid. Halo must close now.\n\nNote: This is a bug.\n", signature);
        if(DEDICATED_SERVER) {
            std::cerr << error;
        }
        else {
            MessageBox(NULL, error, "Chimera Error", MB_OK);
        }
        ExitProcess(135);
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

    extern "C" const char *signature_errors() {
        static char error_buffer[65536];

        std::size_t error_buffer_offset = 0;
        #define APPEND_SPRINTF(...) error_buffer_offset += std::snprintf(error_buffer + error_buffer_offset, sizeof(error_buffer) - error_buffer_offset, __VA_ARGS__)

        APPEND_SPRINTF("Could not load Chimera.\n\n");

        bool engine_type_missing = false;

        auto list_missing_sigs_for_feature = [&error_buffer_offset, &engine_type_missing](const char *feature) {
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

        return error_buffer;
    }

    int halo_type() {
        return chimera->feature_present("client") ? 1 : chimera->feature_present("server") ? 2 : 0;
    }

    extern "C" const std::byte *address_for_signature(const char *signature) {
        return chimera->get_signature(signature).data();
    }

    const char *Chimera::get_path() noexcept {
        if(this->p_path.size() == 0) {
            this->p_path = halo_path();
            char last_char = this->p_path[this->p_path.size() - 1];
            if(last_char != '\\' || last_char != '/') {
                this->p_path += '\\';
            }
            this->p_path += "chimera\\";

            // Make directories
            CreateDirectory(this->p_path.data(), nullptr);
            char maps_directory[MAX_PATH];
            std::snprintf(maps_directory, sizeof(maps_directory), "%s\\maps", this->p_path.data());
            CreateDirectory(maps_directory, nullptr);
        }
        return this->p_path.data();
    }

    void Chimera::reload_config() {
        char config_path[MAX_PATH];
        std::snprintf(config_path, sizeof(config_path), "%s%s", this->get_path(), "preferences.txt");
        this->p_config = std::make_unique<Config>(config_path);
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
            // Set default settings
            if(chimera->get_ini()->get_value_bool("halo.optimal_defaults").value_or(false)) {
                chimera->execute_command("chimera_block_mouse_acceleration true");
                chimera->execute_command("chimera_hud_kill_feed true");
                chimera->execute_command("chimera_aim_assist true");
                chimera->execute_command("chimera_interpolate true");
                chimera->execute_command("chimera_diagonals 0.75");
                chimera->execute_command("chimera_block_loading_screen true");
                chimera->execute_command("chimera_fov auto");
                chimera->execute_command("chimera_widescreen_fix true");
                chimera->execute_command("chimera_throttle_fps 300");
                chimera->execute_command("chimera_uncap_cinematic true");
                if(chimera->feature_present("client_af")) {
                    chimera->execute_command("chimera_af true");
                }
            }

            // Set console enabled
            if(chimera->get_ini()->get_value_bool("halo.console").value_or(false)) {
                set_console_enabled(true);
            }

            // Load the custom chat
            initialize_custom_chat();

            // Fix console fade
            setup_console_fade_fix();

            // Fix the death reset time
            setup_death_reset_time_fix();

            chimera->reload_config();
            setup_text_hook();
        }
        enable_output(true);
    }

    const std::vector<Command> &Chimera::get_commands() const noexcept {
        return this->p_commands;
    }
}
