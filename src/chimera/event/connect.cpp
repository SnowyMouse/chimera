// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "connect.hpp"

extern "C" {
    bool on_preconnect_asm() noexcept;
    const void *continue_preconnect;
}

namespace Chimera {
    static void enable_connect_hook();

    static std::vector<Event<ConnectEventFunction>> preconnect_events;

    void add_preconnect_event(const ConnectEventFunction function, EventPriority priority) {
        // Remove if exists
        remove_preconnect_event(function);

        // Enable camera hook if not enabled
        enable_connect_hook();

        // Add the event
        preconnect_events.emplace_back(Event<ConnectEventFunction> { function, priority });
    }

    void remove_preconnect_event(const ConnectEventFunction function) {
        for(std::size_t i = 0; i < preconnect_events.size(); i++) {
            if(preconnect_events[i].function == function) {
                preconnect_events.erase(preconnect_events.begin() + i);
                return;
            }
        }
    }

    extern "C" bool on_preconnect(std::uint32_t &ip, std::uint16_t &port, char16_t *password) {
        char password_char8[9] = {};
        std::copy(password, password + sizeof(password_char8) - 1, password_char8);
        bool allow = true;
        call_in_order_allow(preconnect_events, allow, ip, port, password_char8);
        return allow;
    }

    /**
     * Enable the connect hook if it's not already enabled.
     */
    static void enable_connect_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_function_override(get_chimera().get_signature("on_connect_sig").data(), hook, reinterpret_cast<const void *>(on_preconnect_asm), &continue_preconnect);
    }
}
