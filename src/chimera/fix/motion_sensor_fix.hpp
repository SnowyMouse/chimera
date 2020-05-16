// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MOTION_SENSOR_FIX_HPP
#define CHIMERA_MOTION_SENSOR_FIX_HPP

namespace Chimera {
    /**
     * Fix motion sensor being fucked at high frame rates
     */
    void set_up_motion_sensor_fix() noexcept;
}

#endif
