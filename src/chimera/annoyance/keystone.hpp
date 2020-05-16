// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_KEYSTONE_HPP
#define CHIMERA_KEYSTONE_HPP

namespace Chimera {
    /**
     * Remove keystone.dll (speeds up load time and makes Halo more stable)
     */
    void remove_keystone() noexcept;
}

#endif
