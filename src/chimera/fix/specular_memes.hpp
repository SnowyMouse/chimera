// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SPECULAR_LIGHT_FIX
#define CHIMERA_SPECULAR_LIGHT_FIX

namespace Chimera {
    /**
     * Meme the specular light draw so it doesn't rely on undefined behaviour to work.
     */
    void set_up_specular_light_fix() noexcept;
}

#endif
