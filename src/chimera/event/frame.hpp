// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FRAME_HPP
#define CHIMERA_FRAME_HPP

#include "event.hpp"

namespace Chimera {
    /**
     * Add or replace a preframe event. This event occurs before the frame.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_preframe_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a preframe event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_preframe_event(const EventFunction function);

    /**
     * Add or replace a frame event. This event occurs after the frame.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_frame_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a frame event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_frame_event(const EventFunction function);
}

#endif
