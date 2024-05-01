// SPDX-License-Identifier: GPL-3.0-only

#include "../../signature/signature.hpp"
#include "../../halo_data/contrail.hpp"
#include "../../halo_data/pause.hpp"
#include "../../halo_data/object.hpp"
#include "../../chimera.hpp"

#include "interpolate.hpp"

#include "contrail.hpp"

namespace Chimera {
    // Basically the object interpolation stuff except hacked at and repurposed

    #define CONTRAIL_BUFFER_SIZE 256
    struct ContrailParent {
        /** Roll back contrail parent object's position data to previous ticks value.*/
        bool rollback = false;

        /** Object ID of the parent object. */
        ObjectID object_id;

        /** Tag ID of the parent object. */
        TagID tag_id;

        /** This is the position of the object's center. */
        Point3D center;

        /** This is the number of nodes this object has. */
        std::size_t node_count;

        /** These are the model nodes used by the object. */
        ModelNode nodes[MAX_NODES];
    };

    // This is the contrail parent object data.
    static ContrailParent object_buffers[2][CONTRAIL_BUFFER_SIZE] = {};

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = object_buffers[0];
    static auto *previous_tick = object_buffers[1];

    // If true, a tick has passed and it's time to re-copy the object data.
    static bool tick_passed = false;

    static void copy_objects() noexcept;

    void fix_contrail_before() noexcept {
        if(game_paused()) {
            return;
        }

        // Check if a tick has passed. If so, swap buffers and copy new objects.
        if(tick_passed) {
            if(current_tick == object_buffers[0]) {
                current_tick = object_buffers[1];
                previous_tick = object_buffers[0];
            }
            else {
                current_tick = object_buffers[0];
                previous_tick = object_buffers[1];
            }

            copy_objects();
            tick_passed = false;
        }

        auto &object_table = ObjectTable::get_object_table();
        auto &contrail_table = ContrailTable::get_contrail_table();
        auto max_objects = contrail_table.current_size;

        for(std::size_t i = 0; i < max_objects && i < CONTRAIL_BUFFER_SIZE; i++) {
            auto &current_tick_object = current_tick[i];
            auto &previous_tick_object = previous_tick[i];

            // Skip if we don't want to bodge projectile positions this frame.
            if(!current_tick_object.rollback) {
                continue;
            }

            auto *object = object_table.get_dynamic_object(current_tick_object.object_id);

            // This shouldn't ever happen but just in case it does...
            if(!object) {
                continue;
            }

            // Hit the contrail with a sledgehammer. I mean if it works, right?...
            if (!previous_tick_object.rollback && object->type == ObjectType::OBJECT_TYPE_PROJECTILE) {
                contrail_table.first_element[i].parent_object_id.whole_id = 0xFFFFFFFF;
                continue;
            }

            // Skip if the tags do not match
            auto &tag_id = object->tag_id;
            if(tag_id != current_tick_object.tag_id || previous_tick_object.tag_id != tag_id) {
                continue;
            }

            // Skip if object ID's do not match. Mostly a sanity check.
            if(current_tick_object.object_id != previous_tick_object.object_id) {
                continue;
            }

            // Copy previous tick positions to object table to fudge contrails
            object->center_position = previous_tick_object.center;
            std::copy(previous_tick_object.nodes, previous_tick_object.nodes + previous_tick_object.node_count, object->nodes());
        }
    }


    // Copy objects from Halo's data to buffer
    static void copy_objects() noexcept {
        // Get the object table
        auto &object_table = ObjectTable::get_object_table();

        // Get the contrail table
        auto &contrail_table = ContrailTable::get_contrail_table();

        // Go through all contrails.
        for(std::size_t i = 0; i < CONTRAIL_BUFFER_SIZE; i++) {
            auto &current_tick_object = current_tick[i];

            // Set this to false so if the parent object doesn't exist or is invalid we don't need to restore position data.
            current_tick_object.rollback = false;

            //Check if contrail exists
            if(contrail_table.first_element[i].id == 0) {
                continue;
            }

            // Store parent Object ID
            current_tick_object.object_id = contrail_table.first_element[i].parent_object_id;

            // See if the parent object exists.
            auto *object = object_table.get_dynamic_object(current_tick_object.object_id);
            if(!object) {
                continue;
            }

            // Check if we can do anything with this object.
            auto *nodes = object->nodes();
            if(!nodes) {
                continue;
            }

            // Get the number of model nodes.
            current_tick_object.tag_id = object->tag_id;
            auto *object_tag = get_tag(current_tick_object.tag_id.index.index);
            if(!object_tag) {
                continue;
            }

            // Get the model tag to get the node count
            if(object->type == ObjectType::OBJECT_TYPE_PROJECTILE) {
                current_tick_object.node_count = 1;
            }
            else {
                const auto &model_tag_id = *reinterpret_cast<const TagID *>(object_tag->data + 0x28 + 0xC);
                auto *model_tag = get_tag(model_tag_id);
                if(!model_tag) {
                    current_tick_object.node_count = 0;
                }
                else {
                    current_tick_object.node_count = *reinterpret_cast<std::uint32_t *>(model_tag->data + 0xB8);
                }
            }

            current_tick_object.rollback = true;

            // Copy nodes from Halo's data
            std::copy(nodes, nodes + current_tick_object.node_count, current_tick_object.nodes);
            current_tick_object.center = object->center_position;
        }
    }

    void fix_contrail_after() noexcept {
        if(game_paused()) {
            return;
        }

        auto &object_table = ObjectTable::get_object_table();
        auto &contrail_table = ContrailTable::get_contrail_table();
        auto max_objects = contrail_table.current_size;

        for(std::size_t i = 0; i < max_objects && i < CONTRAIL_BUFFER_SIZE; i++) {
            auto &current_tick_object = current_tick[i];

            // Skip if we didn't fix the contrails this frame.
            if(!current_tick_object.rollback) {
                continue;
            }

            // Unbodge the gigabodge.
            if (current_tick_object.object_id != contrail_table.first_element[i].parent_object_id && contrail_table.first_element[i].parent_object_id.whole_id == 0xFFFFFFFF) {
                contrail_table.first_element[i].parent_object_id = current_tick_object.object_id;
                continue;
            }

            auto *object = object_table.get_dynamic_object(current_tick_object.object_id);

            // This shouldn't ever happen but just in case it does...
            if(!object) {
                continue;
            }

            object->center_position = current_tick_object.center;
            std::copy(current_tick_object.nodes, current_tick_object.nodes + current_tick_object.node_count, object->nodes());
        }
    }

    void fix_contrail_clear() noexcept {
        // Erase the object buffers to prevent funny things on revert
        std::memset(object_buffers, 0, sizeof(object_buffers));
    }

    void fix_contrail_on_tick() noexcept {
        tick_passed = true;
    }
}
