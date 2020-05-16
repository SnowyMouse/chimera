// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_LIGHT_HPP
#define CHIMERA_LIGHT_HPP

#include <cstdint>

#include "object.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    struct Light;

    struct LightTable : GenericTable<Light> {
        /**
         * Get the light table
         * @return reference to the light table
         */
        static LightTable &get_light_table() noexcept;
    };

    struct Light {
        // 0x0
        std::uint32_t unknown0;
        std::uint32_t some_id;
        std::uint32_t unknown1;
        std::uint32_t some_counter;
        // 0x10
        std::uint32_t unknown2;
        float red;
        float green;
        float blue;
        // 0x20
        std::uint32_t unknown3;
        std::uint32_t unknown4;
        std::uint32_t unknown5;
        std::uint32_t parent_object_id;
        // 0x30
        Point3D position;
        Point3D orientation[2];
        // 0x54
        char i_stopped_caring_at_this_point[0x28];
    };
    static_assert(sizeof(Light) == 0x7C);
}

#endif
