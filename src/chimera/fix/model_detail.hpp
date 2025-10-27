// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MODEL_DETAIL_HPP
#define CHIMERA_MODEL_DETAIL_HPP

namespace Chimera {
    /**
     * Do we want to use the highest LOD level always?
     */
    extern "C" bool force_max_lod_level;

    /**
     * Fix the model detail at resolutions higher than 480p.
     */
    void set_up_model_detail_fix() noexcept;
}

#endif
