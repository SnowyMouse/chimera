// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "camera.hpp"

namespace Chimera {
    static void enable_camera_hook();

    static std::vector<Event<EventFunction>> precamera_events;

    void add_precamera_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_precamera_event(function);

        // Enable camera hook if not enabled
        enable_camera_hook();

        // Add the event
        precamera_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_precamera_event(const EventFunction function) {
        for(std::size_t i = 0; i < precamera_events.size(); i++) {
            if(precamera_events[i].function == function) {
                precamera_events.erase(precamera_events.begin() + i);
                return;
            }
        }
    }

    static std::vector<Event<EventFunction>> camera_events;

    void add_camera_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_camera_event(function);

        // Enable camera hook if not enabled
        enable_camera_hook();

        // Add the event
        camera_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_camera_event(const EventFunction function) {
        for(std::size_t i = 0; i < camera_events.size(); i++) {
            if(camera_events[i].function == function) {
                camera_events.erase(camera_events.begin() + i);
                return;
            }
        }
    }

    static void on_precamera() {
        call_in_order(precamera_events);
    }

    static void on_camera() {
        call_in_order(camera_events);
    }

    /**
     * Enable the camera hook if it's not already enabled.
     */
    static void enable_camera_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("on_camera_sig").data(), hook, reinterpret_cast<const void *>(on_precamera), reinterpret_cast<const void *>(on_camera));
    }
}
