// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_TICK_EVENT_HPP
#define CHIMERA_TICK_EVENT_HPP

#include "../event/event.hpp"
#include <cstdint>

namespace Chimera {
    /**
     * Add or replace a pretick event. This event occurs just before the tick occurs.
     * @param function This is the function to add
     * @param priority This is the priority used to determine call order.
     */
    void add_pretick_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a pretick event if the function is being used as an event.
     * @param function This is the function to remove
     */
    void remove_pretick_event(const EventFunction function);

    /**
     * Add or replace a tick event. This event occurs after the tick occurs.
     * @param function This is the function to add
     * @param priority This is the priority used to determine call order.
     */
    void add_tick_event(const EventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a tick event if the function is being used as an event.
     * @param function This is the function to remove
     */
    void remove_tick_event(const EventFunction function);

    /**
     * Get the tick rate of the game. This does not account for game_speed.
     * @return Return the tick rate.
     */
    float tick_rate() noexcept;

    /**
     * Set the tick rate of the game.
     */
    void set_tick_rate(float new_rate) noexcept;

    /**
     * Get the effective tick rate of the game, accounting for game_speed.
     * @return Return the effective tick rate of the game.
     */
    float effective_tick_rate() noexcept;

    /**
     * Get the current tick count
     * @return tick count
     */
    std::int32_t get_tick_count() noexcept;

    /**
     * Get the current tick time
     * @return tick time
     */
    double get_tick_time() noexcept;

    /**
     * Get the current tick progress
     * @return tick progress
     */
    float get_tick_progress() noexcept;
}

#endif
