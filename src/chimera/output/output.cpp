// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "output.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../custom_chat/custom_chat.hpp"
#include "../console/console.hpp"
#include "../event/rcon_message.hpp"
#include "../chimera.hpp"

namespace Chimera {
    const char *output_prefix = nullptr;

    static bool output_enabled = false;
    void enable_output(bool enabled) noexcept {
        output_enabled = enabled;
    }

    extern "C" void send_rcon_message_asm(std::uint32_t player, const char *message) noexcept;
    void send_rcon_message(int player, const char *message) {
        send_rcon_message_asm(static_cast<std::uint32_t>(player), message);
    }

    extern "C" void console_output_asm(const ConsoleColor &color, const char *message);
    void console_output_raw(const ConsoleColor &color, const char *message) noexcept {
        if(!output_enabled) {
            return;
        }
        char message_copy[256];
        if(output_prefix) {
            std::snprintf(message_copy, sizeof(message_copy), "%s: %s", output_prefix, message);
        }
        else {
            std::strncpy(message_copy, message, sizeof(message_copy) - 1);
        }
        console_output_asm(color, message_copy);
    }

    extern "C" void hud_output_asm(const wchar_t *message);
    void hud_output_raw(const wchar_t *message) noexcept {
        if(!output_enabled) {
            return;
        }
        hud_output_asm(message);
    }
    void hud_output_raw(const char *message) noexcept {
        wchar_t x[256] = {};
        for(std::size_t i = 0; i < sizeof(x) / sizeof(*x) - 1 && *message; i++) {
            x[i] = message[i];
        }
        hud_output_asm(x);
    }

    static bool server_messages_are_blocked = false;

    extern "C" void before_rcon_message() noexcept;
    extern "C" bool on_rcon_message(const char *message) noexcept {
        if (!call_rcon_message_events(message)) {
            return false;
        }
        else if(rcon_used_recently()) {
            return true;
        }
        else if(custom_chat_enabled()) {
            add_server_message(message);
            return false;
        }
        else {
            return !server_messages_are_blocked;
        }
    }

    void set_up_rcon_message_hook() noexcept {
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;
        
        static Hook hook;
        if(get_chimera().feature_present("client_rcon")) {
            write_jmp_call(get_chimera().get_signature("rcon_message_sig").data(), hook, reinterpret_cast<const void *>(before_rcon_message));
        }
    }

    void set_server_messages_blocked(bool blocked) noexcept {
        server_messages_are_blocked = blocked;
    }

    bool server_messages_blocked() noexcept {
        return server_messages_are_blocked;
    }
}
