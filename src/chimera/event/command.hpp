// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_COMMAND_EVENT_HPP
#define CHIMERA_COMMAND_EVENT_HPP

#include "../event/event.hpp"

namespace Chimera {
    /**
     * This is an event that is triggered when a command is executed via the console.
     * @param  command This is the command.
     * @return         Return true to allow the command to pass or false to not.
     */
    using CommandEventFunction = bool (*)(const char *command);

    /**
     * Add or replace a command event. This event occurs just before a command is executed.
     * @param function This is the function to add
     * @param priority This is the priority used to determine call order.
     */
    void add_command_event(const CommandEventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a command event if the function is being used as an event.
     * @param function This is the function to remove
     */
    void remove_command_event(const CommandEventFunction function);
}

#endif
