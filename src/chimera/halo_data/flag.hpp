// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FLAG_HPP
#define CHIMERA_FLAG_HPP

#include <cstdint>

#include "object.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    struct Flag;

    struct FlagTable : GenericTable<Flag> {
        /**
         * Get the flag table.
         * @return Return a reference to the flag table.
         */
        static FlagTable &get_flag_table() noexcept;
    };

    /** This is a part of a flag. */
    struct FlagPart {
        /** Coordinates relative to the world */
        Point3D position;

        /** World units per tick */
        Point3D velocity;
    };
    static_assert(sizeof(FlagPart) == 0x18);

    /** This is a flag cloth object. */
    struct Flag {
        std::uint32_t some_id;
        std::uint32_t unknown0;
        ObjectID parent_object_id;
        std::uint32_t some_id1;
        Point3D position;
        FlagPart parts[241];
        char padding[8];
    };
    static_assert(sizeof(Flag) == 0x16BC);
}

#endif
