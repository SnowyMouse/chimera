// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include <cstring>

#include "../../halo_data/object.hpp"
#include "../../halo_data/camera.hpp"
#include "../../halo_data/pause.hpp"
#include "../../halo_data/player.hpp"
#include "camera.hpp"

#include "../../signature/signature.hpp"
#include "../../chimera.hpp"

namespace Chimera {
    struct InterpolatedCamera {
        CameraType type;
        ObjectID followed_object;
        CameraData data;
    };

    // Camera data
    static InterpolatedCamera camera_buffers[2];

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = camera_buffers + 0;
    static auto *previous_tick = camera_buffers + 1;

    // If true, a tick has passed and it's time to re-copy the camera data.
    static bool tick_passed = false;

    // If true, don't undo the camera (interpolation was skipped this frame)
    static bool skip;

    // Used to force first person rotation interpolation
    extern bool spectate_enabled;

    void interpolate_camera_before() noexcept {
        if(game_paused()) {
            return;
        }

        // Check if a tick has passed. If so, copy data to the current interpolation buffer.
        auto type = camera_type();
        if(tick_passed) {
            // Swap buffers.
            if(current_tick == camera_buffers) {
                current_tick = camera_buffers + 1;
                previous_tick = camera_buffers + 0;
            }
            else {
                current_tick = camera_buffers + 0;
                previous_tick = camera_buffers + 1;
            }

            static auto **followed_object = reinterpret_cast<ObjectID **>(get_chimera().get_signature("followed_object_sig").data() + 10);

            // Copy all data.
            current_tick->data = camera_data();
            current_tick->type = type;
            current_tick->followed_object = **followed_object;

            tick_passed = false;

            // Skip if cinematic camera and is not following an object, or if the previous tick's camera is not following the same object or is the same camera type
            skip = (type == CameraType::CAMERA_CINEMATIC && current_tick->followed_object.is_null()) ||
                   (current_tick->followed_object != previous_tick->followed_object || current_tick->type != previous_tick->type);

            // Lastly, if we're not skipping, check if we went really far too quickly in first person
            if(!skip && type == CameraType::CAMERA_FIRST_PERSON) {
                skip = distance_squared(previous_tick->data.position, current_tick->data.position) > 5.0 * 5.0;
            }
        }

        if(skip) {
            return;
        }

        // Interpolate the camera
        auto &data = camera_data();
        extern float interpolation_tick_progress;

        // If we're in a vehicle, interpolate the rotation
        bool vehicle_first_person = false;
        if(type == CameraType::CAMERA_FIRST_PERSON) {
            auto *player = PlayerTable::get_player_table().get_client_player();
            if(player) {
                auto *object = ObjectTable::get_object_table().get_dynamic_object(player->object_id);
                if(object) {
                    vehicle_first_person = !object->parent.is_null();
                }
            }
        }

        // Don't interpolate rotation if in first person unless we're in a vehicle
        if(type != CameraType::CAMERA_FIRST_PERSON || vehicle_first_person || spectate_enabled) {
            interpolate_point(previous_tick->data.position, current_tick->data.position, data.position, interpolation_tick_progress);
            interpolate_point(previous_tick->data.orientation[0], current_tick->data.orientation[0], data.orientation[0], interpolation_tick_progress);
            interpolate_point(previous_tick->data.orientation[1], current_tick->data.orientation[1], data.orientation[1], interpolation_tick_progress);
        }

        // Otherwise, only interpolate Z in case of elevators or Halo's glitchy crouching
        else {
            data.position.z = previous_tick->data.position.z + (current_tick->data.position.z - previous_tick->data.position.z) * interpolation_tick_progress;
        }
    }

    void interpolate_camera_after() noexcept {
        if(skip || game_paused()) {
            return;
        }

        auto &data = camera_data();
        data.position = current_tick->data.position;
        std::copy(current_tick->data.orientation, current_tick->data.orientation + 1, data.orientation);
    }

    void interpolate_camera_clear() noexcept {
        skip = true;
        std::memset(camera_buffers, 0, sizeof(camera_buffers));
    }

    void interpolate_camera_on_tick() noexcept {
        tick_passed = true;
    }
}
