// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_OUTPUT_HPP
#define CHIMERA_OUTPUT_HPP

#include <cstdio>
#include <windows.h>

namespace Chimera {
    struct ConsoleColor {
        float a;
        float r;
        float g;
        float b;

        /** Color used for headers */
        static ConsoleColor header_color() {
            return { 1.0, 1.0, 0.65, 0.85 };
        }

        /** Color used for bodies */
        static ConsoleColor body_color() {
            return { 1.0, 0.49, 0.78, 0.77 };
        }
    };

    /**
     * Send an rcon message to someone
     * @param player  player to send to
     * @param message message to send
     */
    void send_rcon_message(int player, const char *message);

    /**
     * Set whether or not to enable output
     * @param enabled set to true if output should be enabled
     */
    void enable_output(bool enabled) noexcept;

    /**
     * Display a message of a color
     * @param color   Color to use in the message
     * @param message Message to use
     */
    void console_output_raw(const ConsoleColor &color, const char *message) noexcept;

    /**
     * Display a message of a color
     * @param color  Color to use in the message
     * @param format String format (passed to snprintf)
     * @param args   Additional arguments to pass
     */
    template<typename... Args> void console_output(const ConsoleColor &color, const char *format, Args... args) noexcept {
        char message[256];
        std::snprintf(message, sizeof(message), format, args...);
        console_output_raw(color, message);
    }

    /**
     * Display a message (white)
     * @param format String format (passed to snprintf)
     * @param args   Additional arguments to pass
     */
    template<typename... Args> void console_output(const char *format, Args... args) noexcept {
        console_output(ConsoleColor {1.0, 1.0, 1.0, 1.0}, format, args...);
    }

    /**
     * Display an error (red)
     * @param format String format (passed to snprintf)
     * @param args   Additional arguments to pass
     */
    template<typename... Args> void console_error(const char *format, Args... args) noexcept {
        console_output(ConsoleColor {1.0, 1.0, 0.25, 0.25}, format, args...);
    }

    /**
     * Display a warning (yellow)
     * @param format String format (passed to snprintf)
     * @param args   Additional arguments to pass
     */
    template<typename... Args> void console_warning(const char *format, Args... args) noexcept {
        console_output(ConsoleColor {1.0, 1.0, 1.0, 0.125}, format, args...);
    }

    /**
     * Display a message on the HUD
     * @param message Message to use
     */
    void hud_output_raw(const wchar_t *message) noexcept;

    /**
     * Display a message on the HUD
     * @param message Message to use
     */
    void hud_output_raw(const char *message) noexcept;

    /**
     * Set up the rcon message hook
     */
    void set_up_rcon_message_hook() noexcept;

    /**
     * Set whether or not server messages are blocked
     * @param enabled true if blocked
     */
    void set_server_messages_blocked(bool blocked) noexcept;

    /**
     * Get whether or not server messages are blocked
     * @return true if blocked
     */
    bool server_messages_blocked() noexcept;
}

#endif
