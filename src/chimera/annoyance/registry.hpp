// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_EULA_HPP
#define CHIMERA_EULA_HPP

namespace Chimera {
    /**
     * Remove Halo's annoying checks (speeds up loading, removes registry changes, prevents safe mode from happening after crash)
     */
    void remove_registry_checks() noexcept;
}

#endif
