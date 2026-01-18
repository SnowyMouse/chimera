// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FP_MODEL_FIX
#define CHIMERA_FP_MODEL_FIX

namespace Chimera {
    /**
     * Lock the FoV for the FP model to stock FoV or not.
     */
    extern bool lock_fp_model_fov;

    /**
     * Fix the fp model deceleration animation being tied to framerate
     * Fix the fp model fov being tied to camera fov.
     */
    void set_up_fp_model_fix() noexcept;
}

#endif
