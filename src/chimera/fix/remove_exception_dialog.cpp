// SPDX-License-Identifier: GPL-3.0-only

#include "remove_exception_dialog.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"

namespace Chimera {
    void set_up_remove_exception_dialog() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("gathering_exception_sig").data(), hook, reinterpret_cast<const void *>(+[]() {
            std::fprintf(stderr, "The game crashed. RIP process ;-;\n");
            std::exit(68); // NOT nice
        }));
    }
}
