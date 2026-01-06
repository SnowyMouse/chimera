// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GAMETYPE_INDICATOR_MEMES_HPP
#define CHIMERA_GAMETYPE_INDICATOR_MEMES_HPP

namespace Chimera {
    /**
     * True when the gametype indicator is disabled by chimera_block_gametype_indicator.
     */
    extern bool gametype_indicator_disabled;
    /**
     * Make it so the gametype indicator doesn't draw twice when in a vehicle. Also precache
     * the tagID of the bitmaps so the draw function doesn't search for the tag every frame.
     */
    void set_up_gametype_indicator_fix() noexcept;
}

#endif
