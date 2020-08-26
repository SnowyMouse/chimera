// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "damage.hpp"

namespace Chimera {
    extern "C" void apply_damage_asm(DamageObjectStructThing *damage, std::uint32_t object);

    void apply_damage(ObjectID object, TagID damage_effect, float multiplier, PlayerID causer_player, ObjectID causer_object) noexcept {
        DamageObjectStructThing s;
        s.damage_tag_id = damage_effect;
        s.causer_player = causer_player;
        s.causer_object = causer_object;
        s.multiplier = multiplier;
        apply_damage_asm(&s, object.whole_id);
    }
}
