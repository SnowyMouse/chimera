// SPDX-License-Identifier: GPL-3.0-only

#include "game_engine.hpp"
#include "pause.hpp"

namespace Chimera {
    bool game_paused() noexcept {
        switch(game_engine()) {
            case GameEngine::GAME_ENGINE_DEMO:
                return *reinterpret_cast<bool *>(0x4BAD02EA);
            default:
                return *reinterpret_cast<bool *>(0x400002EA);
        }
    }
}
