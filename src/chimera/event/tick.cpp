// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../math_trig/math_trig.hpp"
#include <optional>

#include "tick.hpp"

namespace Chimera {
    static void enable_tick_hook();

    static LARGE_INTEGER current_tick_time;

    static std::vector<Event<EventFunction>> pretick_events;

    void add_pretick_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_pretick_event(function);

        // Enable tick hook if not enabled
        enable_tick_hook();

        // Add the event
        pretick_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_pretick_event(const EventFunction function) {
        for(std::size_t i = 0; i < pretick_events.size(); i++) {
            if(pretick_events[i].function == function) {
                pretick_events.erase(pretick_events.begin() + i);
                return;
            }
        }
    }

    static std::vector<Event<EventFunction>> tick_events;

    void add_tick_event(const EventFunction function, EventPriority priority) {
        // Remove if exists
        remove_tick_event(function);

        // Enable tick hook if not enabled
        enable_tick_hook();

        // Add the event
        tick_events.emplace_back(Event<EventFunction> { function, priority });
    }

    void remove_tick_event(const EventFunction function) {
        for(std::size_t i = 0; i < tick_events.size(); i++) {
            if(tick_events[i].function == function) {
                tick_events.erase(tick_events.begin() + i);
                return;
            }
        }
    }

    static void on_pretick() {
        call_in_order(pretick_events);
    }

    static void on_tick() {
        QueryPerformanceCounter(&current_tick_time);
        call_in_order(tick_events);
    }

    /**
     * Enable the tick hook if it's not already enabled.
     */
    static void enable_tick_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("on_tick_sig").data(), hook, reinterpret_cast<const void *>(on_pretick), reinterpret_cast<const void *>(on_tick));
    }

    float tick_rate() noexcept {
        static float *tick_ptr = nullptr;
        if(!tick_ptr) {
            tick_ptr = *reinterpret_cast<float **>(get_chimera().get_signature("tick_rate_sig").data() + 2);
        }
        return *tick_ptr;
    }

    void set_tick_rate(float new_rate) noexcept {
        float *tick_ptr = *reinterpret_cast<float **>(get_chimera().get_signature("tick_rate_sig").data() + 2);
        DWORD prota, protb;
        VirtualProtect(tick_ptr, sizeof(tick_ptr), PAGE_READWRITE, &prota);
        *tick_ptr = new_rate;
        VirtualProtect(tick_ptr, sizeof(tick_ptr), prota, &protb);
    }

    float effective_tick_rate() noexcept {
        static const float *game_speed_ptr = nullptr;
        if(!game_speed_ptr) {
            game_speed_ptr = reinterpret_cast<float *>(**reinterpret_cast<std::byte ***>(get_chimera().get_signature("game_speed_sig").data() + 1) + 0x18);
        }
        return *game_speed_ptr * tick_rate();
    }

    std::int32_t get_tick_count() noexcept {
        static std::int32_t *tick_count = nullptr;
        if(!tick_count) {
            tick_count = reinterpret_cast<std::int32_t *>(**reinterpret_cast<std::byte ***>(get_chimera().get_signature("tick_counter_sig").data() + 1) + 0xC);
        }
        return *tick_count;
    }

    double get_tick_time() noexcept {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter_time_elapsed(current_tick_time, counter);
    }

    float get_tick_progress() noexcept {
        static std::optional<float *> tick_progress;
        if(!tick_progress.has_value()) {
            tick_progress = reinterpret_cast<float *>(**reinterpret_cast<std::byte ***>(get_chimera().get_signature("tick_progress_sig").data() + 1) + 304);
        }

        float v = effective_tick_rate() * **tick_progress;

        if(v > 1) {
            v = 1;
        }

        return v;
    }
}
