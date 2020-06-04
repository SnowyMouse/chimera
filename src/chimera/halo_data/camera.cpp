// SPDX-License-Identifier: GPL-3.0-only

#include "camera.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include <optional>

namespace Chimera {
    CameraType camera_type() noexcept {
        static auto *cta = reinterpret_cast<CameraType *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("camera_type_sig").data() + 0x2) + 0x56);
        return *cta;
    }

    CameraData &camera_data() noexcept { //0x647600 usually
        static std::optional<CameraData *> camera_coord_addr;
        if(!camera_coord_addr.has_value()) {
            camera_coord_addr = reinterpret_cast<CameraData *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("camera_coord_sig").data() + 2) - 0x8);
        }
        return **camera_coord_addr;
    }
}
