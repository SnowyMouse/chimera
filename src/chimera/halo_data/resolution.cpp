// SPDX-License-Identifier: GPL-3.0-only

#include "resolution.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    Resolution &get_resolution() noexcept {
        static Resolution *resolution = *reinterpret_cast<Resolution **>(get_chimera().get_signature("resolution_sig").data() + 4);
        return *resolution;
    }
}
