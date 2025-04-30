// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CHICAGO_MULTIPLY_FIX_HPP
#define CHIMERA_CHICAGO_MULTIPLY_FIX_HPP

namespace Chimera {
    /**
     * Fix shader_transparent_chicago failing to render when the framebuffer blend function is set to multiply 
     * while the framebuffer fade mode is set to none
     */
    void set_up_chicago_multiply_fix() noexcept;
}

#endif
