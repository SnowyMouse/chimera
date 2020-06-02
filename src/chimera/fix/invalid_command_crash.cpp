#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

extern "C" {
    void *halo_precompile_script_fn = nullptr;
    void handle_invalid_command_crash_asm() noexcept;
}

namespace Chimera {
    void set_up_invalid_command_crash_fix() noexcept {
        halo_precompile_script_fn = reinterpret_cast<void *>(get_chimera().get_signature("precompile_script_sig").data());
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("invalid_command_script_sig").data() + 9, hook, nullptr, reinterpret_cast<const void *>(handle_invalid_command_crash_asm), false);
    }
}
