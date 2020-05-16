// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_RESOURCE_MAP_HPP
#define CHIMERA_RESOURCE_MAP_HPP

#include <cstdint>

namespace Chimera {
    struct ResourceMapHeader {
        /**
         * Type of resource map
         */
        std::uint32_t type;

        /**
         * Offset to paths
         */
        std::uint32_t paths;

        /**
         * Offset to resource indices
         */
        std::uint32_t resources;

        /**
         * Number of resources
         */
        std::uint32_t resource_count;
    };
    static_assert(sizeof(ResourceMapHeader) == 0x10);

    struct ResourceMapResource {
        /**
         * Resource path offset from tag paths offset
         */
        std::uint32_t path_offset;

        /**
         * Size of resource
         */
        std::uint32_t size;

        /**
         * Ofset of resource data
         */
        std::uint32_t data_offset;
    };
    static_assert(sizeof(ResourceMapResource) == 0xC);
}

#endif
