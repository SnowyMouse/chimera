// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_VCR_HPP
#define CHIMERA_VCR_HPP

#include "event.hpp"

#include <cstdint>

namespace Chimera {
    using VCREventFunction = void (*)(const wchar_t *cassette_command, std::uint8_t player_index);

    /**
     * Add or replace a vcr event. This event occurs upon the cassette tape being inserted
     * @param function This is the function
     * @param priority Priority to use
     */
    void add_vcr_event(const VCREventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove a preframe event if the function is being used as an event.
     * @param function This is the function
     */
    void remove_vcr_event(const VCREventFunction function);
}

#endif
