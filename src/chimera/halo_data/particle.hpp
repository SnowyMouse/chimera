// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HALO_DATA_PARTICLE_HPP
#define CHIMERA_HALO_DATA_PARTICLE_HPP

#include <cstdint>

#include "table.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    struct Particle;

    struct ParticleTable : GenericTable<Particle> {
        /**
         * Get the particle table.
         * @return Return a reference to the particle table.
         */
        static ParticleTable &get_particle_table() noexcept;
    };

    struct Particle {
        // 0x0
        std::uint32_t unknown0;
        std::uint32_t tag_id;
        std::uint32_t unknown1 = 0xFFFFFFFF;
        std::uint32_t unknown2;
        // 0x10
        std::uint32_t frames_alive;
        float a0; // 0.0 = begin to fade out
        float a;
        float b;
        // 0x20
        float c; // idk, don't set this to anything less than or equal to 0 or the game will freeze with max CPU usage somehow
        std::uint32_t unknown3;
        std::uint32_t unknown4;
        std::uint32_t some_id;
        // 0x30
        Point3D position;
        float unknownx0;
        // 0x40
        float unknowny0;
        float unknownz0;
        float unknownx1;
        float unknowny1;
        // 0x50
        float unknownz1;
        float radius_x;
        float radius_y;
        float radius_z;
        // 0x60
        std::uint32_t unknown5;
        float red;
        float green;
        float blue;
    };
    static_assert(sizeof(Particle) == 0x70);
}
#endif
