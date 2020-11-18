// SPDX-License-Identifier: GPL-3.0-only

#include "rcon_message.hpp"

namespace Chimera {
    static std::vector<Event<RconMessageEvent>> rcon_message_events;

	void add_rcon_message_event(const RconMessageEvent function, EventPriority priority) {
        // Remove if exists
        remove_rcon_message_event(function);

        // Enable tick hook if not enabled
        set_up_rcon_message_hook();

        // Add the event
        rcon_message_events.emplace_back(Event<RconMessageEvent> { function, priority });
    }

    void remove_rcon_message_event(const RconMessageEvent function) {
        for(std::size_t i = 0; i < rcon_message_events.size(); i++) {
            if(rcon_message_events[i].function == function) {
                rcon_message_events.erase(rcon_message_events.begin() + i);
                return;
            }
        }
    }

    bool call_rcon_message_events(const char* message) noexcept {
        bool allow = true;
    	call_in_order_allow(rcon_message_events, allow, message);
        return allow;
    }
}