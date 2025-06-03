// SPDX-License-Identifier: GPL-3.0-only

#include "cutscene.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include <optional>

namespace Chimera {
    CinematicGlobals &get_cinematic_globals() noexcept {
        static auto *cin_globals = **reinterpret_cast<CinematicGlobals ***>(get_chimera().get_signature("cinematic_globals_sig").data() + 0x2);
        return *cin_globals;
    }
}
