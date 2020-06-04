// SPDX-License-Identifier: GPL-3.0-only

#include "../../halo_data/antenna.hpp"

#include "antenna.hpp"

#include <algorithm>

namespace Chimera {
    // Antenna data
    #define MAX_ANTENNA 0xC
    static Antenna antenna_buffers[2][MAX_ANTENNA] = {};

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = antenna_buffers[0];
    static auto *previous_tick = antenna_buffers[1];

    // If true, a tick has passed and it's time to re-copy the antenna data.
    static bool tick_passed = false;

    void interpolate_antenna_before() noexcept {
        auto &antenna_table = AntennaTable::get_antenna_table();
        if(tick_passed) {
            // Swap buffers.
            if(current_tick == antenna_buffers[0]) {
                current_tick = antenna_buffers[1];
                previous_tick = antenna_buffers[0];
            }
            else {
                current_tick = antenna_buffers[0];
                previous_tick = antenna_buffers[1];
            }
            tick_passed = false;

            // Copy data
            std::copy(antenna_table.first_element, antenna_table.first_element + MAX_ANTENNA, current_tick);
        }

        for(std::size_t i = 0; i < antenna_table.current_size && i < MAX_ANTENNA; i++) {
            extern float interpolation_tick_progress;
            auto &current_tick_object = current_tick[i];
            auto &previous_tick_object = previous_tick[i];
            auto &object_in_memory = antenna_table.first_element[i];

            // Interpolate each vertex
            interpolate_point(previous_tick_object.position, current_tick_object.position, object_in_memory.position, interpolation_tick_progress);
            for(std::size_t v = 0; v < sizeof(current_tick_object.vertices) / sizeof(current_tick_object.vertices[0]); v++) {
                interpolate_point(previous_tick_object.vertices[v].position, current_tick_object.vertices[v].position, object_in_memory.vertices[v].position, interpolation_tick_progress);
            }
        }
    }

    void interpolate_antenna_after() noexcept {
        auto &antenna_table = AntennaTable::get_antenna_table();
        for(std::size_t i = 0; i < antenna_table.current_size && i < MAX_ANTENNA; i++) {
            auto &current_tick_object = current_tick[i];
            auto &object_in_memory = antenna_table.first_element[i];

            // Uninterpolate everything
            object_in_memory.position = current_tick_object.position;
            for(std::size_t v = 0; v < sizeof(current_tick_object.vertices) / sizeof(current_tick_object.vertices[0]); v++) {
                object_in_memory.vertices[v].position = current_tick_object.vertices[v].position;
            }
        }
    }

    void interpolate_antenna_on_tick() noexcept {
        tick_passed = true;
    }
}
