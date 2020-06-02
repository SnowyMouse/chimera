#include "../config/ini.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../chimera.hpp"
#include "main_menu_music.hpp"

namespace Chimera {
    void block_main_menu_music_if_needed() noexcept {
        if(!get_chimera().get_ini()->get_value_bool("halo.main_menu_music").value_or(true)) {
            LARGE_INTEGER asdf;
            LARGE_INTEGER *path = *reinterpret_cast<LARGE_INTEGER **>(get_chimera().get_signature("main_menu_music_sig").data() + 11);
            QueryPerformanceCounter(&asdf);
            overwrite(path, asdf);
        }
    }
}
