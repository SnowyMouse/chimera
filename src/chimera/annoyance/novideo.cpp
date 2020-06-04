// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    static std::uint32_t *novideo = nullptr;

    static void do_it_do_it() {
        *novideo = 1;
    }

    void enable_novideo() noexcept {
        static Hook hook;
        auto *instruction = get_chimera().get_signature("novideo_sig").data() + 2;
        novideo = *reinterpret_cast<std::uint32_t **>(instruction + 1);
        write_jmp_call(instruction, hook, nullptr, reinterpret_cast<const void *>(do_it_do_it));
    }
}
