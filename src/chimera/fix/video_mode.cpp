#include "video_mode.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

#include <cstdint>

namespace Chimera {
    static bool vsync = false;
    extern "C" void on_set_video_mode_initially() noexcept;

    void set_up_video_mode() noexcept {
        bool pc = chimera.feature_present("client_resolution_pc");
        bool demo = chimera.feature_present("client_resolution_demo");

        if(!pc && !demo) {
            return;
        }

        std::uint32_t default_width = 800;
        std::uint32_t default_height = 600;
        std::uint32_t default_refresh_rate = 60;

        auto &chimera = get_chimera();
        auto *ini = chimera.get_ini();
        auto *enabled = ini->get_value("video_mode.enabled");
        if(!enabled || *enabled != '1') {
            return;
        }

        #define SET_VALUE(key, value) { \
            auto *v = ini->get_value("video_mode." key); \
            if(v) { \
                value = std::stoi(v); \
            } \
        }

        // Load and set values
        SET_VALUE("width", default_width)
        SET_VALUE("height", default_height)
        SET_VALUE("refresh_rate", default_refresh_rate)
        SET_VALUE("vsync", vsync)

        if(pc) {
            auto *default_res = chimera.get_signature("default_resolution_pc_sig").data();
            overwrite(default_res + 4, default_width);
            overwrite(default_res + 12, default_height);
            overwrite(default_res + 20, default_refresh_rate);
        }
        else if(demo) {
            auto *default_res = chimera.get_signature("default_resolution_demo_sig").data();
            overwrite(default_res + 1, default_width);
            overwrite(default_res + 28, default_height);
            overwrite(default_res + 6, default_refresh_rate);
        }

        // Disable Halo's loading of the profile data
        overwrite(chimera.get_signature("load_profile_resolution_sig").data(), static_cast<std::uint8_t>(0xEB));

        // And lastly, intercept Halo setting resolution
        static Hook set_hook;
        write_jmp_call(chimera.get_signature("default_resolution_set_sig").data(), set_hook, reinterpret_cast<const void *>(on_set_video_mode_initially));
    }

    extern "C" void set_vsync_setting_initially(std::byte *data) noexcept {
        if(!vsync) {
            *reinterpret_cast<std::uint32_t *>(data + 0x34) = 0x80000000;
        }
    }
}
