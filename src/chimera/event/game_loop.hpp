// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GAME_LOOP_HPP
#define CHIMERA_GAME_LOOP_HPP

#include "event.hpp"

namespace Chimera {
    /**
     * Add or replace a game start event. This event occurs before the main loop runs but after the game has set up resources.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_game_start_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a game start event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_game_start_event(const EventFunction function);

    /**
     * Add or replace a game exit event. This event occurs after the main loop exits but before the game starts freeing resources.
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_game_exit_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a game exit event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_game_exit_event(const EventFunction function);
}

#endif
