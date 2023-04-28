// SPDX-License-Identifier: GPL-3.0-only

#include "object.hpp"
#include "multiplayer.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"

namespace Chimera {
    ObjectTable &ObjectTable::get_object_table() noexcept {
        static auto &object_table = ***reinterpret_cast<ObjectTable ***>(get_chimera().get_signature("object_table_sig").data() + 2);
        return object_table;
    }

    BaseDynamicObject *ObjectTable::get_dynamic_object(const ObjectID &object_id) noexcept {
        auto *object = this->get_element(object_id.index.index);
        if(object && object->id == object_id.index.id) {
            return object->object;
        }
        else {
            return nullptr;
        }
    }

    BaseDynamicObject *ObjectTable::get_dynamic_object(std::uint32_t index) noexcept {
        auto *object = this->get_element(index);
        if(object) {
            return object->object;
        }
        else {
            return nullptr;
        }
    }

    struct s_object_creation_disposition {
        uint32_t tag_id;
        PAD(0x4);
        uint32_t player_id;
        uint32_t parent;
        PAD(0x4);
        PAD(0x4);
        float pos[3];
    };

    extern "C" {
        void create_object_query_asm(TagID tag_id, ObjectID parent, s_object_creation_disposition *query);
        ObjectID create_object_asm(s_object_creation_disposition *query, std::uint32_t object_type);
        void delete_object_asm(TagID tag_id);
    }

    ObjectID spawn_object(const TagID &tag_id, float x, float y, float z, const ObjectID &parent) noexcept {
        char buffer[1024] = {};
        auto *object_create_query = reinterpret_cast<s_object_creation_disposition *>(buffer);

        // Create query
        create_object_query_asm(tag_id, parent, object_create_query);

        // Set object parameters
        object_create_query->player_id = 0xFFFFFFFF;
        object_create_query->pos[0] = x;
        object_create_query->pos[1] = y;
        object_create_query->pos[2] = z;

        /**
            0 = created by local machine
            1 = host
            2 = ???
            3 = client sided object (from giraffe)
        */
        std::uint32_t object_type = (server_type() == SERVER_LOCAL) ? 0 : 3;

        auto object_id = create_object_asm(object_create_query, object_type);
        return object_id;
    }

    void delete_object(ObjectID object_id) noexcept {
        delete_object_asm(object_id);
    }
}
