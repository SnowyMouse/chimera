// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_WEATHER_FIX_HPP
#define CHIMERA_WEATHER_FIX_HPP

namespace Chimera {
    /**
     * Fix some weather effects being weird at high framerates.
     */
    void set_up_weather_fix() noexcept;
}

#endif
