// SPDX-License-Identifier: GPL-3.0-only

#include "exception_dialog.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/error_box.hpp"
#include "../config/ini.hpp"
#include "../chimera.hpp"

namespace Chimera {
    void set_up_remove_exception_dialog() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("gathering_exception_sig").data(), hook, reinterpret_cast<const void *>(+[]() {
            show_error_box("Segmentation fault", "The game has encountered a segmentation fault.", get_chimera().get_ini()->get_value_bool("error_handling.show_segmentation_fault").value_or(false));
            std::exit(68); // NOT nice
        }));
    }
}
