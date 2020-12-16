// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/output.hpp"
#include "../output/error_box.hpp"
#include <time.h>

namespace Chimera {
    void reduce_drm() noexcept {
        auto &chimera = get_chimera();

        // Disregard the trial
        if(!chimera.feature_present("core_full")) {
            return;
        }

        // Remove the registry check. Speeds up Halo loading and improves Wine compatibility.
        if(chimera.feature_present("client_full")) {
            auto &client_drm_sig = chimera.get_signature("client_drm_sig");
            SigByte client_drm_mod[] = {0xEB, 0x13};
            write_code_s(client_drm_sig.data(), client_drm_mod);
        }

        // Allow invalid keys to join. If we don't, almost nobody can join anyway, so we might as well remove that restriction.
        auto &server_drm_1_sig = chimera.get_signature("server_drm_1_sig");
        SigByte server_drm_mod[] = {0x90, 0x90, 0x90, 0x90, 0x90};
        write_code_s(server_drm_1_sig.data() + 10, server_drm_mod);

        // Allow duplicate keys to join. This is useful for testing mods and scripts.
        auto &server_drm_2_sig = chimera.get_signature("server_drm_2_sig");
        overwrite(server_drm_2_sig.data() + 5, static_cast<std::uint8_t>(0xEB));
    }

    extern "C" void fun_cd_key_hash_function_asm() noexcept;

    void set_cd_hash() noexcept {
        // If we have a hash, set up the custom hash thing. This is to prevent servers from tracking you (and maybe leave a funny message in their logs).
        auto *hash = get_chimera().get_ini()->get_value("halo.hash");
        if(hash && std::strlen(hash) > 0) {
            if(!get_chimera().feature_present("client_edit_cd_hash")) {
                show_error_box("Error", "Using custom CD hashes is not supported on this client. (client is probably modified)");
                std::exit(1);
            }

            static Hook hook;
            write_jmp_call(get_chimera().get_signature("cd_key_hash_sig").data() + 12, hook, nullptr, reinterpret_cast<const void *>(fun_cd_key_hash_function_asm));
        }
    }

    extern "C" void fun_cd_key_hash_gen_asm(std::uint8_t *key) noexcept {
        auto *hash = get_chimera().get_ini()->get_value("halo.hash");
        if(hash) {
            #define HASH_LENGTH 32

            // Randomize the hash
            if(std::strcmp(hash, "%") == 0) {
                srand(time(nullptr));
                char characters[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
                for(std::size_t i = 0; i < HASH_LENGTH; i++) {
                    key[i] = characters[rand() % sizeof(characters)];
                }
            }

            // Use a custom hash and pad out the rest of the hash with '0's.
            else {
                std::memset(key, '0', HASH_LENGTH);
                auto len = std::strlen(hash);
                std::memcpy(reinterpret_cast<char *>(key), hash, len > HASH_LENGTH ? HASH_LENGTH : len);
            }
        }
    }
}
