#include "master_server.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void set_master_server_connection_threads(std::int8_t threads) noexcept {
        overwrite(get_chimera().get_signature("setup_master_server_connection_sig").data() + 10, threads);
    }
}
