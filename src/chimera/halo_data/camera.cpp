#include "camera.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    CameraType camera_type() noexcept {
        static auto *cta = reinterpret_cast<CameraType *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("camera_type_sig").data() + 0x2) + 0x56);
        return *cta;
    }

    CameraData &camera_data() noexcept { //0x647600 usually
        static auto *camera_coord_addr = *reinterpret_cast<CameraData **>(get_chimera().get_signature("camera_coord_sig").data() + 2);
        return *camera_coord_addr;
    }
}
