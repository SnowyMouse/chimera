#include <optional>
#include "script.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" void execute_script_asm(const char *script, std::byte *script_function) noexcept;

    void execute_script(const char *script) noexcept {
        static std::optional<std::byte *> script_function;
        if(!script_function.has_value()) {
            script_function = get_chimera().get_signature("execute_script_sig").data();
        }
        execute_script_asm(script, *script_function);
    }
}
