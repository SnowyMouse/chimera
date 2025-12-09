// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CHICAGO_FIX_HPP
#define CHIMERA_CHICAGO_FIX_HPP

namespace Chimera {
    /**
     * Fix shader_transparent_chicago multiply and multiply2x blend modes. 
     * Fix the shader not performing perspective division when sampling the first map.
     */
    void set_up_chicago_fix() noexcept;
}

#endif
