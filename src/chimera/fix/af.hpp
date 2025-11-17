// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_AF_HPP
#define CHIMERA_AF_HPP

#include <cstddef>
#include <cstdint>

namespace Chimera {
    // Trial has no built in AF, so use this to substitute the flag used in retail/custom.
    extern bool af_trial;
    extern bool *af_is_enabled;
    extern std::uint32_t global_max_anisotropy;

    /**
     * Enable AF for models and decals.
     */
    void set_up_model_af() noexcept;
}

#endif
