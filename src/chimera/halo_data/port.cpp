// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <windows.h>
#include "port.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    static std::uint32_t *get_ports() {
        static std::optional<std::uint32_t *> ports;
        if(!ports.has_value()) {
            ports = *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("server_port_sig").data() + 3);
        }
        return *ports;
    }

    static std::uint16_t &get_client_port_a() {
        return *reinterpret_cast<std::uint16_t *>(&get_ports()[1]);
    }

    static std::uint16_t &get_server_port_a() {
        return *reinterpret_cast<std::uint16_t *>(&get_ports()[0]);
    }

    void set_ports() noexcept {
        auto *set_port_sig = get_chimera().get_signature("set_port_sig").data();
        static constexpr SigByte NOP_CODE[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

        auto client_port_setting = get_chimera().get_ini()->get_value_long("halo.client_port");
        if(client_port_setting.has_value()) {
            get_client_port_a() = static_cast<std::uint16_t>(*client_port_setting);
            write_code_s(set_port_sig + 6, NOP_CODE);
        }

        auto server_port_setting = get_chimera().get_ini()->get_value_long("halo.server_port");
        if(server_port_setting.has_value()) {
            get_server_port_a() = static_cast<std::uint16_t>(*server_port_setting);
            write_code_s(set_port_sig, NOP_CODE);
        }
    }
    std::uint16_t get_client_port() noexcept {
        return get_client_port_a();
    }
    std::uint16_t get_server_port() noexcept {
        return get_server_port_a();
    }
}
