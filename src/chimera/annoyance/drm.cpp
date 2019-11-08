#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void reduce_drm() noexcept {
        auto &chimera = get_chimera();

        if(!chimera.feature_present("core_drm")) {
            return;
        }

        if(chimera.feature_present("client_drm")) {
            auto &client_drm_sig = chimera.get_signature("client_drm_sig");
            SigByte client_drm_mod[] = {0xEB, 0x13};
            write_code_s(client_drm_sig.data(), client_drm_mod);
        }

        auto &server_drm_1_sig = chimera.get_signature("server_drm_1_sig");
        SigByte server_drm_mod[] = {0x90, 0x90, 0x90, 0x90, 0x90};
        write_code_s(server_drm_1_sig.data() + 10, server_drm_mod);

        auto &server_drm_2_sig = chimera.get_signature("server_drm_2_sig");
        overwrite(server_drm_2_sig.data() + 5, static_cast<std::uint8_t>(0xEB));
    }
}
