// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "../config/ini.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../chimera.hpp"
#include "../output/output.hpp"
#include "main_menu_music.hpp"

namespace Chimera {
    static bool always_force = false;

    void block_main_menu_music_if_needed() noexcept {
        if(!get_chimera().get_ini()->get_value_bool("halo.main_menu_music").value_or(true)) {
            always_force = true;
            set_force_block_main_menu_music(true);
        }
    }

    void set_force_block_main_menu_music(bool force) noexcept {
        auto &sig = get_chimera().get_signature("main_menu_music_sig");
        static std::optional<LARGE_INTEGER> old_value = std::nullopt;

        // Always force?
        if(always_force) {
            force = true;
        }

        // Force the thing?
        LARGE_INTEGER *path = *reinterpret_cast<LARGE_INTEGER **>(sig.data() + 11);
        if(!old_value.has_value()) {
            old_value = *path;
        }

        // We're basically just overwriting this with the current timer because I couldn't come up with something else to overwrite it with lol
        LARGE_INTEGER asdf;
        if(force) {
            QueryPerformanceCounter(&asdf);
        }
        else {
            asdf = old_value.value();
        }

        overwrite(path, asdf);
    }
}
