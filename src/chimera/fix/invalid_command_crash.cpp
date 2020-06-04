// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/frame.hpp"
#include "../event/map_load.hpp"
#include "../halo_data/object.hpp"
#include "../halo_data/tag.hpp"
#include "../output/output.hpp"

extern "C" {
    void *halo_get_function_index_fn = nullptr;
    void *halo_get_global_index_fn = nullptr;
    void handle_invalid_command_crash_asm() noexcept;
    void handle_invalid_global_crash_asm() noexcept;

    void should_ignore_broken_globals_asm() noexcept;
    void should_not_ignore_broken_globals_asm() noexcept;
}

namespace Chimera {
    struct ScriptNode {
        std::uint16_t salt, index_union, type, flags;
        HaloID next_node;
        std::uint32_t string_offset;
        std::uint32_t data;
    };
    static_assert(sizeof(ScriptNode) == 0x14);

    static void jason_jones_away_bullshit() {
        auto *scenario_tag_data = get_tag(get_tag_data_header().scenario_tag)->data;

        const auto *string_data = *reinterpret_cast<const char **>(scenario_tag_data + 0x488 + 0xC);
        auto *node_table = reinterpret_cast<GenericTable<ScriptNode> *>(*reinterpret_cast<std::byte **>(scenario_tag_data + 0x474 + 0xC));
        auto *syntax_data = reinterpret_cast<ScriptNode *>(node_table + 1);

        for(std::size_t i = 0; i < node_table->current_size; i++) {
            auto &fn_node = syntax_data[i];

            // Are we a "begin" function
            if(fn_node.type == 2 && std::strcmp(string_data + fn_node.string_offset, "begin") == 0) {
                auto *reference = reinterpret_cast<HaloID *>(&fn_node.next_node);
                while(!reference->is_null()) {
                    auto &this_node = syntax_data[reference->index.index];
                    auto *next_reference = &this_node.next_node;
                    bool blocked = false;

                    auto &call_maybe = *reinterpret_cast<HaloID *>(&this_node.data);
                    if(!call_maybe.is_null() && call_maybe.index.index < node_table->current_size) {
                        auto &call = syntax_data[call_maybe.index.index];
                        if(call.type == 2) {
                            auto *name = string_data + call.string_offset;

                            // Are we running a blocked command?
                            if(std::strncmp(name, "sv_", 3) == 0 || std::strcmp(name, "connect") == 0 || std::strcmp(name, "disconnect") == 0 || std::strcmp(name, "crash") == 0) {
                                blocked = true;
                            }

                            // Are we setting stuff we shouldn't be setting?
                            else if(std::strcmp(name, "set") == 0) {
                                auto variable_id = reinterpret_cast<HaloID *>(&call.next_node);
                                if(!variable_id->is_null() && variable_id->index.index < node_table->current_size) {
                                    auto &variable_node = syntax_data[variable_id->index.index];
                                    auto *variable = string_data + variable_node.string_offset;

                                    if(
                                        std::strcmp(variable, "sv_tk_ban") == 0 ||
                                        std::strcmp(variable, "multiplayer_draw_teammates_names") == 0 ||
                                        std::strcmp(variable, "sv_client_action_queue_limit") == 0 ||
                                        std::strcmp(variable, "sv_client_action_queue_tick_limit") == 0 ||
                                        std::strcmp(variable, "cl_remote_player_action_queue_limit") == 0 ||
                                        std::strcmp(variable, "cl_remote_player_action_queue_tick_limit") == 0
                                    ) {
                                        blocked = true;
                                    }
                                }
                            }
                        }
                    }

                    // If blocked, *replace* the reference to the node with the next node (effectively skipping it). Otherwise, go to the next node
                    if(blocked) {
                        *reference = *next_reference;
                    }
                    else {
                        reference = next_reference;
                    }
                }
            }
        }
    }

    void set_up_invalid_command_crash_fix() noexcept {
        add_map_load_event(jason_jones_away_bullshit);

        static Hook hook_global;
        halo_get_global_index_fn = reinterpret_cast<void *>(get_chimera().get_signature("get_global_index_sig").data());
        write_jmp_call(get_chimera().get_signature("get_global_indices_map_load_sig").data(), hook_global, nullptr, reinterpret_cast<const void *>(handle_invalid_global_crash_asm), false);

        add_preframe_event(should_ignore_broken_globals_asm, EventPriority::EVENT_PRIORITY_BEFORE);
        add_map_load_event(should_not_ignore_broken_globals_asm);
    }
}
