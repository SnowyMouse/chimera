// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "damage.hpp"

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
