// SPDX-License-Identifier: GPL-3.0-only

#include "d3d9_reset.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    static std::vector<Event<ResetEventFunction>> reset_events;

    static void enable_d3d9_reset_hook();

    extern "C" {
        void on_d3d9_reset_asm();
    }

    void add_d3d9_reset_event(const ResetEventFunction function, EventPriority priority) {
        // Remove if exists
        remove_d3d9_reset_event(function);

        // Enable hook if necessary
        enable_d3d9_reset_hook();

        // Add the event
        reset_events.emplace_back(Event<ResetEventFunction> { function, priority });
    }

    void remove_d3d9_reset_event(const ResetEventFunction function) {
        for(std::size_t i = 0; i < reset_events.size(); i++) {
            if(reset_events[i].function == function) {
                reset_events.erase(reset_events.begin() + i);
                return;
            }
        }
    }

    extern "C" void do_d3d9_reset_event(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS *present) {
        call_in_order(reset_events, device, present);
    }

    static void enable_d3d9_reset_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("d3d9_call_reset_sig").data(), hook, reinterpret_cast<const void *>(on_d3d9_reset_asm), nullptr, false);
    }
}
