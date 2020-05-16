// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GAME_ENGINE_HPP
#define CHIMERA_GAME_ENGINE_HPP

namespace Chimera {
    /** Engine used */
    enum GameEngine {
        /** Custom Edition */
        GAME_ENGINE_CUSTOM_EDITION,

        /** Retail */
        GAME_ENGINE_RETAIL,

        /** Demo */
        GAME_ENGINE_DEMO
    };

    /**
     * Get the game engine used
     * @return game engine used
     */
    GameEngine game_engine() noexcept;
}

#endif
