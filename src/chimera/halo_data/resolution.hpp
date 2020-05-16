// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_RESOLUTION_HPP
#define CHIMERA_RESOLUTION_HPP

#include <cstdint>

namespace Chimera {
    /** This is the resolution of the game. */
    struct Resolution {
        /** Height in pixels */
        std::uint16_t height;

        /** Width in pixels */
        std::uint16_t width;
    };

    /**
     * Get the current resolution of Halo.
     * @return Return the current resolution of Halo.
     */
    Resolution &get_resolution() noexcept;
}

#endif
