// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_ANTENNA_HPP
#define CHIMERA_ANTENNA_HPP

#include <cstdint>

#include "object.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    struct Antenna;

    struct AntennaTable : GenericTable<Antenna> {
        /**
         * Get the antenna table.
         * @return Return a reference to the antenna table.
         */
        static AntennaTable &get_antenna_table() noexcept;
    };

    struct AntennaVertex {
        Point3D position;
        Point3D velocity;
        float scale;
        std::uint32_t counter;
    };

    struct Antenna {
        std::uint32_t unknown_0;
        std::uint32_t unknown_1;
        std::uint32_t tag_id;
        std::uint32_t parent_object_id;
        Point3D position;
        AntennaVertex vertices[0x15];
    };
}

#endif
