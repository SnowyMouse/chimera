// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

#include "game_loop.hpp"

namespace Chimera {
    static void enable_game_loop_hook();

    static std::vector<Event<EventFunction>> game_start_events;
    static std::vector<Event<EventFunction>> game_exit_events;

    void add_game_start_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_game_start_event(function);

        // Enable game exit hook if not enabled
        enable_game_loop_hook();

        // Add the event
        game_start_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void add_game_exit_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_game_exit_event(function);

        // Enable game exit hook if not enabled
        enable_game_loop_hook();

        // Add the event
        game_exit_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_game_start_event(const EventFunction function) {
        for(std::size_t i = 0; i < game_start_events.size(); i++) {
            if(game_start_events[i].function == function) {
                game_start_events.erase(game_start_events.begin() + i);
                return;
            }
        }
    }

    void remove_game_exit_event(const EventFunction function) {
        for(std::size_t i = 0; i < game_exit_events.size(); i++) {
            if(game_exit_events[i].function == function) {
                game_exit_events.erase(game_exit_events.begin() + i);
                return;
            }
        }
    }

    static void on_game_start() {
        call_in_order(game_start_events);
    }

    static void on_game_exit() {
        call_in_order(game_exit_events);
    }

    /**
     * Enable the game loop hook if it's not already enabled.
     */
    static void enable_game_loop_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("on_game_loop_sig").data(), hook, reinterpret_cast<const void *>(on_game_start), reinterpret_cast<const void *>(on_game_exit));
    }
}
