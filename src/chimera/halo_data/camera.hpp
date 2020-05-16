// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CAMERA_HALO_DATA_HPP
#define CHIMERA_CAMERA_HALO_DATA_HPP

#include <cstdint>
#include "type.hpp"

namespace Chimera {
    /**
     * This is the state the camera is in.
     */
    enum CameraType : std::uint16_t {
        /** First person view; rendering the player's weapon */
        CAMERA_FIRST_PERSON = 0,

        /** Vehicle view (third person); can be controlled by the player */
        CAMERA_VEHICLE,

        /** Cinematic camera */
        CAMERA_CINEMATIC,

        /** Devcam or death cam */
        CAMERA_DEBUG
    };

    /**
     * Get the camera type.
     * @return Return the camera type.
     */
    CameraType camera_type() noexcept;

    struct CameraData {
        /** This is the position of the camera. */
        Point3D position;

        /** More stuff */
        std::uint32_t unknown[5];

        /** Orientation/rotation of the camera. */
        Point3D orientation[2];

        /** FOV (radians) */
        float fov;
    };
    static_assert(sizeof(CameraData) == 0x3C);

    /**
     * Get the camera data.
     * @return Return a reference to the camera data.
     */
    CameraData &camera_data() noexcept;
}


#endif
