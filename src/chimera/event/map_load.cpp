// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/game_engine.hpp"

#include "map_load.hpp"

namespace Chimera {
    static void enable_map_load_hook();

    static std::vector<Event<EventFunction>> map_load_events;

    void add_map_load_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_map_load_event(function);

        // Enable frame hook if not enabled
        enable_map_load_hook();

        // Add the event
        map_load_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_map_load_event(const EventFunction function) {
        for(std::size_t i = 0; i < map_load_events.size(); i++) {
            if(map_load_events[i].function == function) {
                map_load_events.erase(map_load_events.begin() + i);
                return;
            }
        }
    }

    static void on_map_load() {
        call_in_order(map_load_events);
    }

    /**
     * Enable the frame hook if it's not already enabled.
     */
    static void enable_map_load_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        auto &chimera = get_chimera();
        if(chimera.feature_present("server")) {
            write_jmp_call(chimera.get_signature(game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION ? "on_map_load_server_custom_sig" : "on_map_load_server_retail_sig").data(), hook, nullptr, reinterpret_cast<const void *>(on_map_load));
        }
        else {
            write_jmp_call(chimera.get_signature("on_map_load_client_sig").data(), hook, reinterpret_cast<const void *>(on_map_load));
        }
    }
}
