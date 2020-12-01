// SPDX-License-Identifier: GPL-3.0-only

#include "video_mode.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/frame.hpp"

#include <cstdint>

namespace Chimera {
    static bool vsync = false;
    extern "C" {
        void on_set_video_mode_initially() noexcept;
        void on_windowed_check_force_windowed() noexcept;
        std::uint32_t force_windowed_mode = 0;
    }

    void set_up_video_mode() noexcept {
        auto &chimera = get_chimera();
        bool pc = chimera.feature_present("client_resolution_pc");
        bool demo = chimera.feature_present("client_resolution_demo");

        if(!pc && !demo) {
            return;
        }

        std::uint32_t default_width = 800;
        std::uint32_t default_height = 600;
        std::uint32_t default_refresh_rate = 60;

        auto *ini = chimera.get_ini();
        if(!ini->get_value_bool("video_mode.enabled").value_or(false)) {
            return;
        }

        int metric = 0;
        auto auto_or_i = [&metric](const char *w) {
            if(std::strcmp(w,"auto") == 0 || std::strcmp(w,"0") == 0) {
                return GetSystemMetrics(metric);
            }
            else {
                return std::stoi(w);
            }
        };

        #define SET_VALUE(key, value, fn) { \
            auto *v = ini->get_value("video_mode." key); \
            if(v) { \
                value = fn(v); \
            } \
        }

        // Load and set values
        metric = SM_CXSCREEN;
        SET_VALUE("width", default_width, auto_or_i)
        metric = SM_CYSCREEN;
        SET_VALUE("height", default_height, auto_or_i)
        SET_VALUE("refresh_rate", default_refresh_rate, std::stoi)
        SET_VALUE("vsync", vsync, std::stoi)

        // Don't fallback the resolution to 800x600
        auto fallback_resolution_sig = chimera.get_signature("fallback_resolution_sig").data();
        static const constexpr SigByte remove_fallback_resolution[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(fallback_resolution_sig, remove_fallback_resolution);

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

            // Prevent the LAA patch from overruling this
            auto *default_res_override = chimera.get_signature("default_resolution_override_demo_sig").data();
            overwrite(default_res_override + 10, static_cast<std::uint8_t>(0xEB));
        }

        // Disable Halo's loading of the profile data
        overwrite(chimera.get_signature("load_profile_resolution_sig").data(), static_cast<std::uint8_t>(0xEB));

        // And lastly, intercept Halo setting resolution
        static Hook set_hook;
        write_jmp_call(chimera.get_signature("default_resolution_set_sig").data(), set_hook, reinterpret_cast<const void *>(on_set_video_mode_initially));

        // Also, windowed mode
        static Hook hook;
        auto *windowed_sig = chimera.get_signature("windowed_sig").data();
        write_jmp_call(windowed_sig, hook, reinterpret_cast<const void *>(on_windowed_check_force_windowed), nullptr, false);
        force_windowed_mode = ini->get_value_bool("video_mode.windowed").value_or(false);
    }

    static void set_borderless_window() noexcept {
        HWND window;
        HMONITOR monitor;
        MONITORINFO monitor_info;

        // Get our window
        window = GetActiveWindow();
        if(!window) {
            goto cleanup;
        }

        // Query monitor information
        monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        monitor_info.cbSize = sizeof(monitor_info);
	    if(!GetMonitorInfo(monitor, &monitor_info)) {
            goto cleanup;
        }

        // Work our magic!
        ShowWindow(window, SW_HIDE);
        SetWindowLong(window, GWL_STYLE, 0);
        SetWindowPos(window, NULL, 0, 0, monitor_info.rcMonitor.right, monitor_info.rcMonitor.bottom, 0);

        cleanup:
        remove_preframe_event(set_borderless_window);
    }

    extern "C" void now_set_borderless_windowed_mode() noexcept {
        if(get_chimera().get_ini()->get_value_bool("video_mode.borderless").value_or(false)) {
            add_preframe_event(set_borderless_window);
        }
    }

    extern "C" void set_vsync_setting_initially(std::byte *data) noexcept {
        if(!vsync) {
            *reinterpret_cast<std::uint32_t *>(data + 0x34) = 0x80000000;
        }
    }
}
