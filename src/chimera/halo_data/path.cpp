// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "path.hpp"

namespace Chimera {
    const char *halo_path() noexcept {
        static const char *path = nullptr;
        if(!path) {
            path = *reinterpret_cast<const char **>(get_chimera().get_signature("path_sig").data() + 1);
        }
        return path;
    }
}
