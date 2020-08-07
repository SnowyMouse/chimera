// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA__EVENT__D3D9_END_SCENE_HPP
#define CHIMERA__EVENT__D3D9_END_SCENE_HPP

#include <d3d9.h>

#include "../event/event.hpp"

namespace Chimera {
    /**
     * This is an event that is triggered just before EndScene is called.
     * @param  device This is the device.
     */
    using EndSceneEventFunction = void (*)(LPDIRECT3DDEVICE9 device);

    /**
     * Add or replace an EndScene event. This event occurs just before EndScene is executed.
     * @param function This is the function to add
     * @param priority This is the priority used to determine call order.
     */
    void add_d3d9_end_scene_event(const EndSceneEventFunction function, EventPriority priority = EventPriority::EVENT_PRIORITY_DEFAULT);

    /**
     * Remove an EndScene event if the function is being used as an event.
     * @param function This is the function to remove
     */
    void remove_d3d9_end_scene_event(const EndSceneEventFunction function);
}

#endif
