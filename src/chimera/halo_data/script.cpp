// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "script.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" void execute_script_asm(const char *script, std::byte *script_function) noexcept;

    void execute_script(const char *script, bool lower) noexcept {
        static std::optional<std::byte *> script_function;
        static std::uint8_t *do_not_lowercase_script_addr;
        if(!script_function.has_value()) {
            script_function = get_chimera().get_signature("execute_script_sig").data();
            do_not_lowercase_script_addr = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("do_not_lowercase_script_sig").data() + 1);
        }
        std::uint8_t value_before = *do_not_lowercase_script_addr;
        *do_not_lowercase_script_addr = !lower;
        execute_script_asm(script, *script_function);
        *do_not_lowercase_script_addr = value_before;
    }
}
