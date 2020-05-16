// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CAMERA_HPP
#define CHIMERA_CAMERA_HPP

#include "event.hpp"

namespace Chimera {
    /**
     * Add or replace a precamera event. This event occurs before the camera is read.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_precamera_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a precamera event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_precamera_event(const EventFunction function);

    /**
     * Add or replace a camera event. This event occurs after the camera is read.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_camera_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a camera event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_camera_event(const EventFunction function);
}

#endif
