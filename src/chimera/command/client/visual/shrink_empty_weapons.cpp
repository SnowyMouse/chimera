// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../event/frame.hpp"
#include "../../../halo_data/object.hpp"

namespace Chimera {
    static void shrink_empty_weapons() {
        // Get the object table
        auto &object_table = ObjectTable::get_object_table();

        // Go through all objects.
        for(std::size_t i = 0; i < object_table.max_elements; i++) {
            // See if the object exists.
            auto *object = object_table.get_dynamic_object(i);
            if(!object) {
                continue;
            }

            // Check if the object isn't visible or is not a weapon or is being held
            if(object->no_collision || object->type != ObjectType::OBJECT_TYPE_WEAPON || !object->parent.is_null()) {
                continue;
            }

            // Get the weapon and check if it's empty
            WeaponDynamicObject &weapon = *reinterpret_cast<WeaponDynamicObject *>(object);

            // Get the tag
            auto *tag = get_tag(weapon.tag_id);
            auto *tag_data = tag->data;

            // First check if the age is maxed. If it is, it's empty no matter what. Otherwise, check the weapon ammo.
            if(weapon.age < 1.0F) {
                // First check if empty
                bool empty = true;
                for(auto &magazine : weapon.magazines) {
                    if(magazine.ammo || magazine.loaded_ammo) {
                        empty = false;
                        break;
                    }
                }

                // Since it's not empty, continue
                if(!empty) {
                    continue;
                }

                // Next, let's see if this weapon, at all, uses battery or "must be readied"
                auto &flags = *reinterpret_cast<std::uint32_t *>(tag_data + 0x308);
                if(flags & (1 << 3)) {
                    continue; // must be readied - obviously a flag or oddball
                }

                // Now, check the triggers to see if they use battery or does not require ammo
                bool uses_age = false;
                bool uses_rounds = false;
                auto trigger_count = *reinterpret_cast<std::uint32_t *>(tag_data + 0x4FC);
                auto trigger_data = *reinterpret_cast<std::byte **>(tag_data + 0x4FC + 4);
                for(std::uint32_t i = 0; i < trigger_count; i++) {
                    auto *trigger = trigger_data + i * 0x114;
                    auto &age_per_shot = *reinterpret_cast<float *>(trigger + 0xBC);
                    auto &rounds_per_shot = *reinterpret_cast<std::uint32_t *>(trigger + 0x22);

                    if(age_per_shot > 0.0F) {
                        uses_age = true;
                        break;
                    }

                    if(rounds_per_shot) {
                        uses_rounds = true;
                    }
                }

                // If it doesn't use rounds or it uses age, continue
                if(!uses_rounds || uses_age) {
                    continue;
                }
            }

            // Next, get the model.
            auto *model_tag = get_tag(*reinterpret_cast<TagID *>(tag_data + 0x28 + 0xC));
            if(!model_tag) {
                continue;
            }
            auto node_count = *reinterpret_cast<std::uint32_t *>(model_tag->data + 0xB8);

            // Shrink it!
            auto *nodes = object->nodes();
            for(std::uint32_t i = 0; i < node_count; i++) {
                nodes[i].scale = 0.5F;
            }
        }
    }

    bool shrink_empty_weapons_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(new_enabled) {
                    add_preframe_event(shrink_empty_weapons);
                }
                else {
                    remove_preframe_event(shrink_empty_weapons);
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
