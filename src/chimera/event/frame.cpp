// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

#include "frame.hpp"

namespace Chimera {
    static void enable_frame_hook();

    static std::vector<Event<EventFunction>> preframe_events;

    void add_preframe_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_preframe_event(function);

        // Enable frame hook if not enabled
        enable_frame_hook();

        // Add the event
        preframe_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_preframe_event(const EventFunction function) {
        for(std::size_t i = 0; i < preframe_events.size(); i++) {
            if(preframe_events[i].function == function) {
                preframe_events.erase(preframe_events.begin() + i);
                return;
            }
        }
    }

    static std::vector<Event<EventFunction>> frame_events;

    void add_frame_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_frame_event(function);

        // Enable frame hook if not enabled
        enable_frame_hook();

        // Add the event
        frame_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_frame_event(const EventFunction function) {
        for(std::size_t i = 0; i < frame_events.size(); i++) {
            if(frame_events[i].function == function) {
                frame_events.erase(frame_events.begin() + i);
                return;
            }
        }
    }

    static void on_preframe() {
        call_in_order(preframe_events);
    }

    static void on_frame() {
        call_in_order(frame_events);
    }

    /**
     * Enable the frame hook if it's not already enabled.
     */
    static void enable_frame_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("on_frame_sig").data(), hook, reinterpret_cast<const void *>(on_preframe), reinterpret_cast<const void *>(on_frame));
    }
}
