#include <windows.h>
#include <iostream>
#include <cstring>
#include <cmath>
#include "bookmark/bookmark.hpp"
#include "custom_chat/custom_chat.hpp"
#include "config/config.hpp"
#include "console/console.hpp"
#include "event/frame.hpp"
#include "halo_data/path.hpp"
#include "output/draw_text.hpp"
#include "output/output.hpp"
#include "signature/hook.hpp"
#include "signature/signature.hpp"
#include "chimera.hpp"
#include "annoyance/drm.hpp"
#include "annoyance/keystone.hpp"
#include "annoyance/registry.hpp"
#include "master_server/master_server.hpp"
#include "fast_load/fast_load.hpp"
#include "fix/blue_32bit_color_fix.hpp"
#include "fix/aim_assist.hpp"
#include "fix/death_reset_time.hpp"
#include "fix/descope_fix.hpp"
#include "fix/extend_limits.hpp"
#include "fix/fov_fix.hpp"
#include "fix/leak_descriptors.hpp"
#include "fix/nav_numbers.hpp"
#include "fix/sun_fix.hpp"
#include "fix/video_mode.hpp"
#include "fix/model_detail.hpp"
#include "memory/decompress.hpp"
#include "memory/memory.hpp"
#include "config/ini.hpp"

namespace Chimera {
    static Chimera *chimera;
    static void initial_tick();

    Chimera::Chimera() : p_signatures(find_all_signatures()) {
        chimera = this;

        // If we *can* load Chimera, then do it
        if(find_signatures()) {
            this->p_commands = Chimera::get_all_commands();
            initialize_console_hook();

            // Enable fast loading
            if(this->feature_present("core_fast_load")) {
                initialize_fast_load();
            }

            // Remove the annoying DRM from the game
            remove_drm();

            // Load the ini
            chimera->reload_ini();

            // Fix this
            set_up_fix_leaking_descriptors();

            // Set up memory/decompression stuff
            if(this->feature_present("core_memory")) {
                set_up_chimera_memory();
            }

            if(this->feature_present("client")) {
                // Fix the camo by default
                extern bool camo_fix_command(int argc, const char **argv);
                const char *value_true = "true";
                camo_fix_command(1, &value_true);
                add_preframe_event(initial_tick);

                // Set up this hook
                set_up_rcon_message_hook();

                // Prevent some annoying DLLs and checks from being loaded
                remove_keystone();
                remove_registry_checks();

                // Speed up getting connections from the master server
                set_master_server_connection_threads(50);

                // Fix maps that go above some of Halo's limits
                set_up_extend_limits();

                // Set video mode
                set_up_video_mode();

                // Fix the damn FOV
                set_up_fov_fix();

                // And this should get fixed, too. Holy shit.
                set_up_descope_fix();

                // Why is this broken?
                if(chimera->feature_present("client_widescreen_custom_edition")) {
                    set_up_aim_assist_fix();
                }

                // Last I checked, not even MCC has this fixed lol.
                set_up_sun_fix();

                // Or this?
                set_up_model_detail_fix();

                // This could be useful
                set_up_server_history();

                // lol
                set_up_nav_numbers_fix();

                // More lol
                set_up_blue_32bit_color_fix();
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

        // Look for the superfeature first
        if(std::strncmp(feature, "client_", 7) == 0) {
            if(std::strncmp(feature, "client_", 7) == 0) {
                if(!feature_present("client")) {
                    return false;
                }
            }
        }

        // Check for it now
        bool feature_found = false;
        for(auto &signature : this->p_signatures) {
            if(std::strcmp(signature.feature(), feature) == 0) {
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
            this->p_commands = Chimera::get_all_commands();
        }
    }

    Chimera &get_chimera() {
        return *chimera;
    }

    CHIMERA_EXTERN void instantiate_chimera() {
        chimera = new Chimera();
    }

    CHIMERA_EXTERN void destroy_chimera() {
        delete chimera;
    }

    #define SECONDARY_FEATURE (halo_type() == 2 ? "server" : "client")

    CHIMERA_EXTERN int find_signatures() {
        return chimera->feature_present("client") || chimera->feature_present("server");
    }

    CHIMERA_EXTERN const char *signature_errors() {
        static char error_buffer[65536];

        std::size_t error_buffer_offset = 0;
        #define APPEND_SPRINTF(...) error_buffer_offset += std::snprintf(error_buffer + error_buffer_offset, sizeof(error_buffer) - error_buffer_offset, __VA_ARGS__)

        APPEND_SPRINTF("Could not load Chimera.\n\n");

        auto list_missing_sigs_for_feature = [&error_buffer_offset](const char *feature) {
            auto missing_sigs = chimera->missing_signatures_for_feature(feature);
            if(missing_sigs.size() == 0) {
                APPEND_SPRINTF("Missing signatures from %s.\n      None\n\n", feature);
            }
            else {
                APPEND_SPRINTF("Missing signatures from %s:\n", feature);
                for(auto &sig : missing_sigs) {
                    APPEND_SPRINTF("    - %s\n", sig);
                }
                APPEND_SPRINTF("\n");
            }
        };

        list_missing_sigs_for_feature("core");
        list_missing_sigs_for_feature("client");
        list_missing_sigs_for_feature("server");

        return error_buffer;
    }

    CHIMERA_EXTERN int halo_type() {
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

            // Do things if we successfully made a directory
            if(CreateDirectory(this->p_path.data(), nullptr)) {
                // Create directories like lua
            }
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
            chimera->execute_command("chimera_block_mouse_acceleration true");
            chimera->execute_command("chimera_hud_kill_feed true");
            chimera->execute_command("chimera_enable_console true");
            chimera->execute_command("chimera_aim_assist true");
            chimera->execute_command("chimera_interpolate true");
            chimera->execute_command("chimera_diagonals 0.75");

            // Load the custom chat
            if(chimera->feature_present("client_widescreen_custom_edition")) {
                initialize_custom_chat();
            }

            // Fix console fade
            setup_console_fade_fix();

            // Fix the death reset time
            setup_death_reset_time_fix();

            chimera->reload_config();
            setup_text_hook();
        }
        enable_output(true);
    }
}
