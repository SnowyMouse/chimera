// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FP_MODEL_FIX
#define CHIMERA_FP_MODEL_FIX

#include "../halo_data/rasterizer_common.hpp"

namespace Chimera {
    /**
     * Lock the FoV for the FP model to stock FoV or not.
     */
    extern bool lock_fp_model_fov;

    /**
     * Modified frustum for fp model draw. The fog screen effect needs to be aware of these.
     */
    extern RenderFrustum frustum, frustum_fp;

    /**
     * Fix the fp model deceleration animation being tied to framerate
     * Fix the fp model fov being tied to camera fov.
     */
    void set_up_fp_model_fix() noexcept;
}

#endif
