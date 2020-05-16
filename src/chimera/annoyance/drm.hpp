// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_DRM_HPP
#define CHIMERA_DRM_HPP

namespace Chimera {
    /**
     * Make Halo's annoying DRM less annoying (required in order for Chimera's strings.dll to function properly)
     *
     * Note: This does NOT crack the game, as you need a valid CD key to install/update the game.
     */
    void reduce_drm() noexcept;

    /**
     * Set the CD hash
     */
    void set_cd_hash() noexcept;
}

#endif
