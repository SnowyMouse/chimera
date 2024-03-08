// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

#include "revert.hpp"

namespace Chimera {
    static void enable_revert_hook();

    static std::vector<Event<EventFunction>> revert_events;

    void add_revert_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_revert_event(function);

        // Enable frame hook if not enabled
        enable_revert_hook();

        // Add the event
        revert_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_revert_event(const EventFunction function) {
        for(std::size_t i = 0; i < revert_events.size(); i++) {
            if(revert_events[i].function == function) {
                revert_events.erase(revert_events.begin() + i);
                return;
            }
        }
    }

    static void on_revert() {
        call_in_order(revert_events);
    }

    /**
     * Enable the frame hook if it's not already enabled.
     */
    static void enable_revert_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        auto &chimera = get_chimera();
        write_jmp_call(chimera.get_signature("revert_sig").data() + 0xA, hook, reinterpret_cast<const void *>(on_revert));
    }
}
