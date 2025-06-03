// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CUTSCENE_HALO_DATA_HPP
#define CHIMERA_CUTSCENE_HALO_DATA_HPP

#include <cstdint>
#include "type.hpp"

namespace Chimera {
    
    struct CinematicGlobals {
        float letter_box_size;
        uint32_t letter_box_last_game_time;
        bool showing_letter_box;
        bool cinematic_in_progress;
        bool cinematic_skip_in_progress;
        bool cinematic_suppress_bsp_object_creation;
    };
    static_assert(sizeof(CinematicGlobals) == 0xC);

    /**
     * Get the cinematic globals.
     * @return Return a reference to the cinematic globals.
     */
    CinematicGlobals &get_cinematic_globals() noexcept;
}


#endif
