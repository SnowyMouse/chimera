#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/frame.hpp"
#include "../event/map_load.hpp"

extern "C" {
    void *halo_get_function_index_fn = nullptr;
    void *halo_get_global_index_fn = nullptr;
    void handle_invalid_command_crash_asm() noexcept;
    void handle_invalid_global_crash_asm() noexcept;

    void should_ignore_broken_globals_asm() noexcept;
    void should_not_ignore_broken_globals_asm() noexcept;
}

namespace Chimera {
    void set_up_invalid_command_crash_fix() noexcept {
        static Hook hook_fn, hook_global;
        halo_get_function_index_fn = reinterpret_cast<void *>(get_chimera().get_signature("get_function_index_sig").data());
        halo_get_global_index_fn = reinterpret_cast<void *>(get_chimera().get_signature("get_global_index_sig").data());
        write_jmp_call(get_chimera().get_signature("get_function_indices_map_load_sig").data(), hook_fn, nullptr, reinterpret_cast<const void *>(handle_invalid_command_crash_asm), false);
        write_jmp_call(get_chimera().get_signature("get_global_indices_map_load_sig").data(), hook_global, nullptr, reinterpret_cast<const void *>(handle_invalid_global_crash_asm), false);

        add_preframe_event(should_ignore_broken_globals_asm, EventPriority::EVENT_PRIORITY_BEFORE);
        add_map_load_event(should_not_ignore_broken_globals_asm);
    }
}
