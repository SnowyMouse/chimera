// SPDX-License-Identifier: GPL-3.0-only

#include "command.hpp"

namespace Chimera {
    std::vector<Event<CommandEventFunction>> command_events;

    void add_command_event(const CommandEventFunction function, EventPriority priority) {
        // Remove if exists
        remove_command_event(function);

        // Add the event
        command_events.emplace_back(Event<CommandEventFunction> { function, priority });
    }

    void remove_command_event(const CommandEventFunction function) {
        for(std::size_t i = 0; i < command_events.size(); i++) {
            if(command_events[i].function == function) {
                command_events.erase(command_events.begin() + i);
                return;
            }
        }
    }
}
