// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_EVENT_MAP_LOAD_HPP
#define CHIMERA_EVENT_MAP_LOAD_HPP

#include "event.hpp"

namespace Chimera {
    /**
     * Add or replace a map load event. This event occurs once a map is loaded.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_map_load_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a map load event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_map_load_event(const EventFunction function);
}

#endif
