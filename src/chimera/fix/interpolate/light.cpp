// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>

#include "../../halo_data/light.hpp"

#include "light.hpp"

namespace Chimera {
    // Light data
    #define MAX_LIGHT 0x380
    struct InterpolatedLight {
        bool interpolate = false;
        Point3D position;
        Point3D orientation[2];
        std::uint32_t some_counter = 0;
    };
    static InterpolatedLight light_buffers[2][MAX_LIGHT];

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = light_buffers[0];
    static auto *previous_tick = light_buffers[1];

    // If true, a tick has passed and it's time to re-copy the light data.
    static bool tick_passed = false;

    void interpolate_light_before() noexcept {
        auto &light_table = LightTable::get_light_table();
        if(tick_passed) {
            // Swap buffers.
            if(current_tick == light_buffers[0]) {
                current_tick = light_buffers[1];
                previous_tick = light_buffers[0];
            }
            else {
                current_tick = light_buffers[0];
                previous_tick = light_buffers[1];
            }
            tick_passed = false;

            // Copy data
            for(size_t i = 0; i < MAX_LIGHT; i++) {
                current_tick[i].interpolate = false;
                auto *light = light_table.get_element(i);
                if(!light) {
                    continue;
                }
                auto &current_tick_object = current_tick[i];
                current_tick_object.some_counter = light->some_counter;
                if(current_tick_object.some_counter > previous_tick[i].some_counter) {
                    current_tick[i].interpolate = true;
                    current_tick[i].position = light->position;
                    current_tick[i].orientation[0] = light->orientation[0];
                    current_tick[i].orientation[1] = light->orientation[1];
                }
            }
        }

        // Interpolate
        for(size_t i = 0; i < light_table.current_size && i < MAX_LIGHT; i++) {
            extern float interpolation_tick_progress;
            auto &current_tick_object = current_tick[i];
            auto &previous_tick_object = previous_tick[i];
            auto &tick_object_in_memory = light_table.first_element[i];
            if(current_tick_object.interpolate && previous_tick_object.interpolate) {
                interpolate_point(previous_tick_object.orientation[0], current_tick_object.orientation[0], tick_object_in_memory.orientation[0], interpolation_tick_progress);
                interpolate_point(previous_tick_object.orientation[1], current_tick_object.orientation[1], tick_object_in_memory.orientation[1], interpolation_tick_progress);
                interpolate_point(previous_tick_object.position, current_tick_object.position, tick_object_in_memory.position, interpolation_tick_progress);
            }
        }
    }

    void interpolate_light_clear() noexcept {
        std::memset(light_buffers, 0, sizeof(light_buffers));
    }

    void interpolate_light_on_tick() noexcept {
        tick_passed = true;
    }
}
