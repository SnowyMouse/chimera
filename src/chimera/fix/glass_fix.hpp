// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GLASS_FIX_HPP
#define CHIMERA_GLASS_FIX_HPP

namespace Chimera {
    /**
     * shader_transparent_glass is broken in weird and wonderful ways. This fixes that.
     */
    void set_up_glass_fix() noexcept;
}

#endif
