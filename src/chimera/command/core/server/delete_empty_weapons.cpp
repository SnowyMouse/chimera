// SPDX-License-Identifier: GPL-3.0-only

#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../event/tick.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../halo_data/multiplayer.hpp"

namespace Chimera {
    static void on_tick() noexcept {
        if(server_type() == ServerType::SERVER_LOCAL) {
            auto &object_table = ObjectTable::get_object_table();
            for(std::size_t i = 0; i < object_table.current_size; i++) {
                // First, does the object exist?
                auto *object = object_table.get_dynamic_object(i);
                if(!object) {
                    continue;
                }

                // Next, is it a weapon?
                if(object->type != ObjectType::OBJECT_TYPE_WEAPON) {
                    continue;
                }

                // Next, is this owned by anything?
                if(!object->parent.is_null()) {
                    continue;
                }

                // Is the object disabled?
                if(object->no_collision) {
                    continue;
                }

                // Is the object in motion
                if(distance_squared(object->velocity, Point3D {}) > 0.00001F) {
                    continue;
                }

                // Convert it to a weapon
                auto *weapon = reinterpret_cast<WeaponDynamicObject *>(object);

                // Next, is it empty?
                bool empty_battery = weapon->age >= 1.0;
                bool empty_ammo = true;
                for(auto &mag : weapon->magazines) {
                    empty_ammo = empty_ammo && !mag.ammo && !mag.loaded_ammo;
                }

                // Next, let's check the tag data
                auto *tag = get_tag(object->tag_id);

                // Is it a flag or oddball?
                if(*reinterpret_cast<std::uint32_t *>(tag->data) & (1 << 3)) {
                    continue;
                }

                // Next does it have any triggers?
                auto trigger_count = *reinterpret_cast<std::uint32_t *>(tag->data + 0x4FC);
                if(!trigger_count) {
                    continue;
                }

                // Next, does it have any magazines?
                auto magazine_count = *reinterpret_cast<std::uint32_t *>(tag->data + 0x4F0);

                // Next, does it use battery or ammo?
                bool has_ammo = magazine_count > 0;
                bool uses_battery = false;
                bool uses_ammo = false;
                auto *first_trigger = *reinterpret_cast<std::byte **>(tag->data + 0x4FC + 4);
                for(std::uint32_t t = 0; t < trigger_count; t++) {
                    auto *trigger = first_trigger + t * 276;

                    uses_battery = uses_battery || *reinterpret_cast<float *>(trigger + 0xBC) > 0.0F;
                    if(has_ammo) {
                        uses_ammo = uses_ammo || *reinterpret_cast<std::uint16_t *>(trigger + 0x22) > 0;
                    }
                }

                // This weapon is perpetually empty and shouldn't be deleted then
                if(!uses_battery && !uses_ammo) {
                    continue;
                }

                // We can delete it if its battery is empty or its ammo is empty
                if((!uses_battery || empty_battery) && (empty_ammo || !uses_ammo)) {
                    delete_object(object->full_object_id());
                }
            }
        }
    }

    bool delete_empty_weapons_command(int argc, const char **argv) noexcept {
        static bool active = false;

        if(argc) {
            bool new_active = STR_TO_BOOL(*argv);
            if(new_active != active) {
                active = new_active;
                if(!active) {
                    remove_tick_event(on_tick);
                }
                else {
                    add_tick_event(on_tick);
                }
            }
        }

        console_output(BOOL_TO_STR(active));
        return true;
    }
}
