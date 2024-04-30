// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HALO_DATA_CONTRAIL_HPP
#define CHIMERA_HALO_DATA_CONTRAIL_HPP

#include <cstdint>

#include "table.hpp"
#include "pad.hpp"
#include "object.hpp"

namespace Chimera {
    struct Contrail;

    struct ContrailTable : GenericTable<Contrail> {
        /**
         * Get the contrail table.
         * @return Return a reference to the contrail table.
         */
        static ContrailTable &get_contrail_table() noexcept;
    };

    struct Contrail {
        std::uint16_t id;
        std::uint16_t unknown;
        std::uint32_t tag_id;
        ObjectID parent_object_id;

        // Dont care about the rest really.
        PAD(0x38);
    };
    static_assert(sizeof(Contrail) == 0x44);
}
#endif
