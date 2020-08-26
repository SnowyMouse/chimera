// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA__EVENT__DAMAGE_HPP
#define CHIMERA__EVENT__DAMAGE_HPP

#include "../event/event.hpp"
#include "../halo_data/player.hpp"

namespace Chimera {
    /**
     * This is an event that is triggered upon damage being dealt.
     * @param object         object ID being damaged
     * @param damage_effect  damage_effect tag being used
     * @param multiplier     damage multiplier
     * @param causing_player player doing damage, or null if no player is responsible
     * @param causing_object object doing damage, or null if no object is responsible
     * @returns              true if damage should be dealt
     */
    using DamageEventFunction = bool (*)(ObjectID &object, TagID &damage_effect, float &multiplier, PlayerID &causing_player, ObjectID &causing_object);

    /**
     * Add or replace a damage event. This event occurs upon damage being dealt.
     * @param function This is the function to add
     * @param priority This is the priority used to determine call order.
     */
    void add_damage_event(const DamageEventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a damage event if the function is being used as an event.
     * @param function This is the function to remove
     */
    void remove_damage_event(const DamageEventFunction function);

    /**
     * Set whether or not damage events are bypassed
     * @param bypass should bypass
     */
    void set_bypass_damage_events(bool bypass) noexcept;

    /**
     * Get whether or not damage events are bypassed
     * @return true if bypassing damage events
     */
    bool get_bypass_damage_events() noexcept;
}

#endif
