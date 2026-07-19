// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_ENVIRONMENT_FOG_HPP
#define CHIMERA_ENVIRONMENT_FOG_HPP

#include "rasterizer.hpp"

namespace Chimera {
    /**
    * Inject the missing code for handling screen fog.
    */
    void set_up_screen_fog() noexcept;
}

#endif
