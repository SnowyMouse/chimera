// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_TYPE_HPP
#define CHIMERA_TYPE_HPP

#include <cstdint>
#include "../math_trig/math_trig.hpp"
#include "pad.hpp"

namespace Chimera {
    using TickCount = std::uint32_t;

    union HaloID {
        std::uint32_t whole_id;
        struct {
            std::uint16_t index;
            std::uint16_t id;
        } index;

        static HaloID null_id() noexcept {
            return { 0xFFFFFFFF };
        }

        /**
         * Check if the tag is null (0xFFFFFFFF)
         * @return true if the tag is null
         */
        bool is_null() const noexcept {
            return *this == null_id();
        }

        bool operator==(const HaloID &other) const noexcept {
            return this->whole_id == other.whole_id;
        }

        bool operator!=(const HaloID &other) const noexcept {
            return this->whole_id != other.whole_id;
        }
    };
    static_assert(sizeof(HaloID) == 4);

    using TagID = HaloID;
    using ObjectID = HaloID;
    using PlayerID = HaloID;
}

#endif
