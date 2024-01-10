// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_EVENT_REVERT_HPP
#define CHIMERA_EVENT_REVERT_HPP

#include "event.hpp"

namespace Chimera {
    /**
     * Add or replace a revert event. This event occurs before the reverting (or core loading, or restarting level, or loading a new level etc).
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_revert_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a revert event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_revert_event(const EventFunction function);
}

#endif
