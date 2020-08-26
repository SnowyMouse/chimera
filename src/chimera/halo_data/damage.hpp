// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_DAMAGE_HPP
#define CHIMERA_DAMAGE_HPP

#include "object.hpp"
#include "player.hpp"

namespace Chimera {
    struct DamageObjectStructThing {
        TagID damage_tag_id;
        std::uint32_t flags = 0;
        PlayerID causer_player;
        ObjectID causer_object;
        std::uint32_t unknown1 = 0;
        std::uint32_t unknown2 = 0x0000FFFF;
        std::uint32_t unknown3 = 0;
        std::uint32_t unknown4 = 0x0000FFFF;
        std::uint32_t unknown5 = 0x0000FFFF;
        std::byte padding1[0x1C] = {};

        float modifier = 1.0F;
        float multiplier = 1.0F;

        std::byte padding2[0x8] = {};
    };
    static_assert(sizeof(DamageObjectStructThing) == 0x50);

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
