// SPDX-License-Identifier: GPL-3.0-only

#include "../../halo_data/object.hpp"
#include "../../halo_data/flag.hpp"

#include <cstddef>
#include <cstring>
#include <string>

#include "flag.hpp"

namespace Chimera {
    // Flag data
    #define MAX_FLAG 0x2
    struct InterpolatedFlag {
        // If the flag cloth has moved for a set amount of ticks, interpolate it.
        int counter = 0;

        // This is the flag data to interpolate.
        Flag flag_data;
    };
    static InterpolatedFlag flag_buffers[2][MAX_FLAG];

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = flag_buffers[0];
    static auto *previous_tick = flag_buffers[1];

    // If true, a tick has passed and it's time to re-copy the flag data.
    static bool tick_passed = false;

    void interpolate_flag_before() noexcept {
        auto &flags = FlagTable::get_flag_table();

        if(tick_passed) {
            // Swap buffers.
            if(current_tick == flag_buffers[0]) {
                current_tick = flag_buffers[1];
                previous_tick = flag_buffers[0];
            }
            else {
                current_tick = flag_buffers[0];
                previous_tick = flag_buffers[1];
            }

            tick_passed = false;

            auto &object_table = ObjectTable::get_object_table();

            // Copy flag data into memory
            for(std::size_t i = 0; i < MAX_FLAG; i++) {
                auto &current_tick_object = current_tick[i];
                auto &previous_tick_object = previous_tick[i];

                auto *flag = flags.get_element(i);
                if(flag) {
                    current_tick_object.flag_data = *flag;
                    auto *flag_object = object_table.get_dynamic_object(flag->parent_object_id);

                    // Flag doesn't exist
                    if(!flag_object) {
                        current_tick_object.counter = 0;
                        previous_tick_object.counter = 0;
                        continue;
                    }

                    bool flag_moved = false;
                    for(std::size_t f = 0; f < sizeof(flag->parts) / sizeof(flag->parts[0]); f++) {
                        if(distance_squared(previous_tick_object.flag_data.parts[f].position, current_tick_object.flag_data.parts[f].position) > 0.00001) {
                            flag_moved = true;
                            break;
                        }
                    }

                    // Flag moved. Increase the counter.
                    if(flag_moved) {
                        current_tick_object.counter++;
                        previous_tick_object.counter++;
                    }
                    else {
                        current_tick_object.counter = 0;
                        previous_tick_object.counter = 0;
                    }
                }
                else {
                    current_tick_object.counter = 0;
                    previous_tick_object.counter = 0;
                }
            }
        }

        // Interpolate flags
        for(std::size_t i = 0; i < flags.current_size && i < MAX_FLAG; i++) {
            auto &current_tick_object = current_tick[i];
            auto &previous_tick_object = previous_tick[i];

            if(current_tick_object.counter < 3) {
                continue;
            }

            auto *flag = flags.get_element(i);
            if(flag) {
                extern float interpolation_tick_progress;
                for(std::size_t f = 0; f < sizeof(flag->parts) / sizeof(flag->parts[0]); f++) {
                    interpolate_point(previous_tick_object.flag_data.parts[f].position, current_tick_object.flag_data.parts[f].position, flag->parts[f].position, interpolation_tick_progress);
                }
            }
        }
    }

    void interpolate_flag_clear() noexcept {
        std::memset(flag_buffers, 0, sizeof(flag_buffers));
    }

    void interpolate_flag_on_tick() noexcept {
        tick_passed = true;
    }
}
