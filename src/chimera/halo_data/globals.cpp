// SPDX-License-Identifier: GPL-3.0-only

#include <cstdint>
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "tag.hpp"
#include "globals.hpp"

namespace Chimera {
    ScriptingGlobal read_global(const char *global_name) noexcept {
        static auto *haddr = get_chimera().get_signature("hs_globals_sig").data();
        ScriptingGlobal sg;
        if(haddr) {
            auto *hs_globals = *reinterpret_cast<std::byte **>(haddr + 7);
            auto &first_global = *reinterpret_cast<std::uint32_t *>(haddr + 1);
            auto *hs_globals_table = *reinterpret_cast<std::byte **>(hs_globals);
            auto *hs_table = *reinterpret_cast<std::byte **>(hs_globals_table + 0x34);

            auto *scenario_tag = get_tag(0);
            auto &globals_count = *reinterpret_cast<std::uint32_t *>(scenario_tag->data + 0x4A8);
            auto *globals_address = *reinterpret_cast<char **>(scenario_tag->data + 0x4A8 + 4);

            for(std::uint32_t i = 0; i < globals_count; i++) {
                auto *global = globals_address + i * 92;
                if(strcmp(global, global_name) == 0) {
                    auto &global_type = *reinterpret_cast<std::uint16_t *>(global + 0x20);
                    auto *location = hs_table + (i + first_global) * 8;
                    switch(global_type) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 9:
                        case 10:
                            sg.type = SCRIPTING_GLOBAL_UNIMPLEMENTED;
                            break;
                        case 5:
                            sg.type = SCRIPTING_GLOBAL_BOOLEAN;
                            break;
                        case 6:
                            sg.type = SCRIPTING_GLOBAL_REAL;
                            break;
                        case 7:
                            sg.type = SCRIPTING_GLOBAL_SHORT;
                            break;
                        case 8:
                            sg.type = SCRIPTING_GLOBAL_LONG;
                            break;
                        default:
                            sg.type = SCRIPTING_GLOBAL_LONG;
                            break;
                    }
                    sg.value.long_int = *reinterpret_cast<long *>(location + 4);
                }
            }
        }
        return sg;
    }

    bool set_global(const char *global_name, ScriptingGlobalValue value) noexcept {
        static auto *haddr = get_chimera().get_signature("hs_globals_sig").data();
        if(haddr) {
            auto *hs_globals = *reinterpret_cast<std::byte **>(haddr + 7);
            auto &first_global = *reinterpret_cast<std::uint32_t *>(haddr + 1);
            auto *hs_globals_table = *reinterpret_cast<std::byte **>(hs_globals);
            auto *hs_table = *reinterpret_cast<std::byte **>(hs_globals_table + 0x34);

            auto *scenario_tag = get_tag(0);
            auto &globals_count = *reinterpret_cast<std::uint32_t *>(scenario_tag->data + 0x4A8);
            auto *globals_address = *reinterpret_cast<char **>(scenario_tag->data + 0x4A8 + 4);

            for(std::uint32_t i = 0; i < globals_count; i++) {
                auto *global = globals_address + i * 92;
                if(strcmp(global, global_name) == 0) {
                    auto &global_type = *reinterpret_cast<std::uint16_t *>(global + 0x20);
                    auto *location = hs_table + (i + first_global) * 8;
                    switch(global_type) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 9:
                        case 10:
                            return false;
                            break;
                        case 5:
                            *reinterpret_cast<char *>(location + 4) = value.boolean ? 1 : 0;
                            break;
                        case 6:
                            *reinterpret_cast<float *>(location + 4) = value.real;
                            break;
                        case 7:
                            *reinterpret_cast<short *>(location + 4) = value.short_int;
                            break;
                        default:
                            *reinterpret_cast<long *>(location + 4) = value.long_int;
                            break;
                    }
                    return true;
                }
            }
        }
        return false;
    }
}