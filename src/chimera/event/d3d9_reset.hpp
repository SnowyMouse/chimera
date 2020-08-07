// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA__EVENT__D3D9_RESET_HPP
#define CHIMERA__EVENT__D3D9_RESET_HPP

#include <d3d9.h>

#include "../event/event.hpp"

namespace Chimera {
    /**
     * This is an event that is triggered just before Reset is called.
     * @param  device  This is the device.
     * @param  present This is the present parameters.
     */
    using ResetEventFunction = void (*)(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS *present);

    /**
     * Add or replace an Reset event. This event occurs just before Reset is executed.
     * @param function This is the function to add
     * @param priority This is the priority used to determine call order.
     */
    void add_d3d9_reset_event(const ResetEventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove an Reset event if the function is being used as an event.
     * @param function This is the function to remove
     */
    void remove_d3d9_reset_event(const ResetEventFunction function);
}

#endif
