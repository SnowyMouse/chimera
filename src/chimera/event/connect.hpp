// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CONNECT_HPP
#define CHIMERA_CONNECT_HPP

#include <cstdint>

#include "event.hpp"

namespace Chimera {
    using ConnectEventFunction = bool (*)(std::uint32_t &ip, std::uint16_t &port, const char *password);

    /**
     * Add or replace a preconnect event. This event occurs before a connection is made.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_preconnect_event(const ConnectEventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a preconnect event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_preconnect_event(const ConnectEventFunction function);
}

#endif
