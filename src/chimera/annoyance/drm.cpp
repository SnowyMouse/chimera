#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/output.hpp"

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

    extern "C" void fun_cd_key_hash_function_asm() noexcept;

    void set_cd_hash() noexcept {
        auto *hash = get_chimera().get_ini()->get_value("halo.hash");
        if(hash && std::strlen(hash) > 0) {
            static Hook hook;
            write_jmp_call(get_chimera().get_signature("cd_key_hash_sig").data() + 12, hook, nullptr, reinterpret_cast<const void *>(fun_cd_key_hash_function_asm));
        }
    }

    extern "C" void fun_cd_key_hash_gen_asm(std::uint8_t *key) noexcept {
        auto *hash = get_chimera().get_ini()->get_value("halo.hash");
        if(hash) {
            std::memset(key, '0', 32);
            auto len = std::strlen(hash);
            std::memcpy(reinterpret_cast<char *>(key), hash, len > 32 ? 32 : len);
        }
    }
}
