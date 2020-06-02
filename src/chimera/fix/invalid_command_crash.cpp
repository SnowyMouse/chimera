#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

extern "C" {
    void *halo_get_function_index_fn = nullptr;
    void handle_invalid_command_crash_asm() noexcept;
}

namespace Chimera {
    void set_up_invalid_command_crash_fix() noexcept {
        halo_get_function_index_fn = reinterpret_cast<void *>(get_chimera().get_signature("get_function_index_sig").data());
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("get_function_indices_map_load_sig").data(), hook, nullptr, reinterpret_cast<const void *>(handle_invalid_command_crash_asm), false);
    }
}
