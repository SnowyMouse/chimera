// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_DAMAGE_HPP
#define CHIMERA_DAMAGE_HPP

#include "object.hpp"
#include "player.hpp"

namespace Chimera {
    /**
     * Apply damage to the given object
     * @param object        object to damage
     * @param damage_effect damage effect to apply
     * @param multiplier    multiply damage by this
     * @param causer_player player responsible for the damage
     * @param causer_object object responsible for the damage
     */
    void apply_damage(ObjectID object, TagID damage_effect, float multiplier = 1.0F, PlayerID causer_player = PlayerID::null_id(), ObjectID causer_object = ObjectID::null_id()) noexcept;
}

#endif
