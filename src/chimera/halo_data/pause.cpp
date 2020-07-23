// SPDX-License-Identifier: GPL-3.0-only

#include "pause.hpp"

namespace Chimera {
    bool game_paused() noexcept {
        return *reinterpret_cast<bool *>(0x400002EA);
    }
}
