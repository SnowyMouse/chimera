// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INTERNAL_SHADERS
#define CHIMERA_INTERNAL_SHADERS

namespace Chimera {
    /**
     * Replace shaders loaded from games default shader collection files with fixed ones
     * located internally within strings.dll
     */
    void set_up_internal_shaders() noexcept;
}

#endif
