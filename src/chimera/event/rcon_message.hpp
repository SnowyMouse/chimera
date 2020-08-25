// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_RCON_MESSAGE_HPP
#define CHIMERA_RCON_MESSAGE_HPP

#include "../output/output.hpp"
#include "event.hpp"

namespace Chimera {
    using RconMessageEvent = bool (*)(const char *);

    /**
     * Add or replace a rcon message event. This event occurs after a RCON command.
     * @param function This is the function to add
     * @param priority This is the priority used to determine call order.
     */
    void add_rcon_message_event(const RconMessageEvent function, EventPriority priority);

    /**
     * Remove a rcon message event if the function is being used as an event.
     * @param function This is the function to remove
     */
    void remove_rcon_message_event(const RconMessageEvent function);

    /**
     * Calls the RCON Message events
    */
    bool call_rcon_message_events(const char *message) noexcept;
}

#endif