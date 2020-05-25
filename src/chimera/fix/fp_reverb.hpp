// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FP_REVERB_FIX
#define CHIMERA_FP_REVERB_FIX

namespace Chimera {
    /**
     * Fix the reverberation not working in FP sounds
     */
    void set_up_fp_reverb_fix() noexcept;

    /**
     * Unfix the reverberation not working in FP sounds
     */
    void disable_fp_reverb_fix() noexcept;
}

#endif
