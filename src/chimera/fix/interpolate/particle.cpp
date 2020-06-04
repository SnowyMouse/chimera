// SPDX-License-Identifier: GPL-3.0-only

#include "../../halo_data/particle.hpp"

#include "particle.hpp"

namespace Chimera {
    struct InterpolatedParticle {
        bool interpolate;
        Point3D position;
    };

    #define PARTICLE_BUFFER_SIZE 1024
    static InterpolatedParticle particle_buffers[2][PARTICLE_BUFFER_SIZE] = {};

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = particle_buffers[0];
    static auto *previous_tick = particle_buffers[1];

    // If true, a tick has passed and it's time to re-copy the particle data.
    static bool tick_passed = false;

    void interpolate_particle() noexcept {
        auto &particle_table = ParticleTable::get_particle_table();
        if(tick_passed) {
            // Swap buffers.
            if(current_tick == particle_buffers[0]) {
                current_tick = particle_buffers[1];
                previous_tick = particle_buffers[0];
            }
            else {
                current_tick = particle_buffers[0];
                previous_tick = particle_buffers[1];
            }

            // Go through each particle, determining if any can be interpolated.
            for(std::size_t i = 0; i < PARTICLE_BUFFER_SIZE; i++) {
                auto *particle = particle_table.get_element(i);
                auto &current_tick_particle = current_tick[i];
                current_tick_particle.interpolate = false;

                if(!particle) {
                    continue;
                }

                // Copy the original particle data
                current_tick_particle.position = particle->position;

                // I'm not entirely sure what unknown0 does, but it magically determines if I should interpolate the particle.
                current_tick_particle.interpolate = particle->unknown0 & 0xFFFF;
            }

            tick_passed = false;
        }

        // Iterate through each particle
        for(std::size_t i = 0; i < particle_table.current_size && i < PARTICLE_BUFFER_SIZE; i++) {
            auto *particle = particle_table.first_element + i;
            auto &current_tick_particle = current_tick[i];
            auto &previous_tick_particle = previous_tick[i];
            extern float interpolation_tick_progress;

            // Interpolate each particle that can be interpolated.
            if(current_tick_particle.interpolate && previous_tick_particle.interpolate) {
                interpolate_point(previous_tick_particle.position, current_tick_particle.position, particle->position, interpolation_tick_progress);
            }
        }
    }

    void interpolate_particle_after() noexcept {
        auto &particle_table = ParticleTable::get_particle_table();
        for(std::size_t i = 0; i < particle_table.current_size && i < PARTICLE_BUFFER_SIZE; i++) {
            auto *particle = particle_table.get_element(i);
            auto &current_tick_particle = current_tick[i];
            auto &previous_tick_particle = previous_tick[i];

            // Restore each position.
            if(current_tick_particle.interpolate && previous_tick_particle.interpolate) {
                particle->position = current_tick_particle.position;
            }
        }
    }

    void interpolate_particle_clear() noexcept {
        for(std::size_t i = 0; i < PARTICLE_BUFFER_SIZE; i++) {
            current_tick[i].interpolate = false;
            previous_tick[i].interpolate = false;
        }
    }

    void interpolate_particle_on_tick() noexcept {
        tick_passed = true;
    }
}
