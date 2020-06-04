// SPDX-License-Identifier: GPL-3.0-only

#include "object.hpp"
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

    extern "C" {
        void delete_object_asm(std::uint32_t whole_id);
        void *delete_object_fn = nullptr;
    }

    void delete_object(ObjectID object_id) noexcept {
        if(!delete_object_fn) {
            delete_object_fn = get_chimera().get_signature("delete_object_sig").data() - 10;
        }
        delete_object_asm(object_id.whole_id);
    }
}
