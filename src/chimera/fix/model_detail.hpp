// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MODEL_DETAIL_HPP
#define CHIMERA_MODEL_DETAIL_HPP

namespace Chimera {
    /**
     * Fix the model detail at resolutions higher than 480p.
     */
    void set_up_model_detail_fix() noexcept;

    /**
     * Set the model detail multiplier
     * @param detail detail multiplier
     */
    void set_model_detail(float detail) noexcept;

    /**
     * Get the model detail multiplier
     * @return detail detail multiplier
     */
    float get_model_detail() noexcept;
}

#endif
