// SPDX-License-Identifier: GPL-3.0-only

// Pls don't kill the ASM code
#pragma GCC optimize ("O0")

#include <cstdint>
#include "multiplayer.hpp"
#include "object.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"

namespace Chimera {
    struct s_object_creation_disposition {
        uint32_t tag_id;
        uint32_t unknown0; // can fuck up the shit apparently
        uint32_t player_id;
        uint32_t parent;
        char unknown1[4];
        char unknown2[4];
        float pos[3];
    };

    ObjectID spawn_object(const TagID &tag_id, float x, float y, float z, const ObjectID &parent) noexcept {
        auto &chimera = get_chimera();
        static auto *query_fn = chimera.get_signature("create_object_query_sig").data() - 6;
        static auto *spawn_object_fn = chimera.get_signature("create_object_sig").data() - 24;

        char query[1024] = {};
        asm (
            "pushad;"
            "push %0;"
            "push %1;"
            "lea eax, dword ptr ds:[%2];"
            "call %3;"
            "add esp, 8;"
            "popad;"
            :
            : "r" (parent.whole_id), "r" (tag_id.whole_id), "r" (query), "r" (query_fn)
        );

        auto *sq = reinterpret_cast<s_object_creation_disposition *>(query);
        sq->pos[0] = x;
        sq->pos[1] = y;
        sq->pos[2] = z;
        sq->player_id = 0xFFFFFFFF;

        /**
            0 = created by local machine
            1 = host
            2 = ???
            3 = client sided object (from giraffe)
        */
        std::uint32_t object_type = (server_type() == SERVER_LOCAL) ? 0 : 3;

        ObjectID object_id;
        asm(
            "pushad;"
            "push %3;"
            "lea eax, dword ptr ds:[%1];"
            "push eax;"
            "call %2;"
            "add esp, 8;"
            "mov %0, eax;"
            "popad;"
            : "=m" (object_id.whole_id)
            : "r" (query), "r" (spawn_object_fn), "r" (object_type)
        );
        return object_id;
    }
}
