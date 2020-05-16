// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_LEAK_DESCRIPTORS_HPP
#define CHIMERA_LEAK_DESCRIPTORS_HPP

namespace Chimera {
    /**
     * Fix leaking descriptors with maps that have file size values >128 MiB
     */
    void set_up_fix_leaking_descriptors() noexcept;
}

#endif
