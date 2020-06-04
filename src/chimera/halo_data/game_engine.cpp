// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "game_engine.hpp"

namespace Chimera {
    GameEngine game_engine() noexcept {
        static auto *game_engine = *reinterpret_cast<const char **>(get_chimera().get_signature("game_engine_sig").data() + 4);
        static std::optional<GameEngine> game_engine_used;
        if(!game_engine_used.has_value()) {
            if(std::strcmp(game_engine, "halom") == 0) {
                game_engine_used = GameEngine::GAME_ENGINE_CUSTOM_EDITION;
            }
            else if(std::strcmp(game_engine, "halor") == 0) {
                game_engine_used = GameEngine::GAME_ENGINE_RETAIL;
            }
            else if(std::strcmp(game_engine, "halod") == 0) {
                game_engine_used = GameEngine::GAME_ENGINE_DEMO;
            }
        }
        return game_engine_used.value();
    }
}
