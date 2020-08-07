// SPDX-License-Identifier: GPL-3.0-only

#include "d3d9_end_scene.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    static std::vector<Event<EndSceneEventFunction>> end_scene_events;

    static void enable_d3d9_end_scene_hook();

    extern "C" {
        void on_d3d9_end_scene_asm();
    }

    void add_d3d9_end_scene_event(const EndSceneEventFunction function, EventPriority priority) {
        // Remove if exists
        remove_d3d9_end_scene_event(function);

        // Enable hook if necessary
        enable_d3d9_end_scene_hook();

        // Add the event
        end_scene_events.emplace_back(Event<EndSceneEventFunction> { function, priority });
    }

    void remove_d3d9_end_scene_event(const EndSceneEventFunction function) {
        for(std::size_t i = 0; i < end_scene_events.size(); i++) {
            if(end_scene_events[i].function == function) {
                end_scene_events.erase(end_scene_events.begin() + i);
                return;
            }
        }
    }

    extern "C" void do_d3d9_end_scene_event(LPDIRECT3DDEVICE9 device) {
        call_in_order(end_scene_events, device);
    }

    static void enable_d3d9_end_scene_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("d3d9_call_end_scene_sig").data(), hook, reinterpret_cast<const void *>(on_d3d9_end_scene_asm), nullptr, false);
    }
}
