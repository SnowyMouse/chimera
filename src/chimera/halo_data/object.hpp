// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_OBJECT_HPP
#define CHIMERA_OBJECT_HPP

#include <cstddef>
#include <cstdint>

#include "../math_trig/math_trig.hpp"
#include "pad.hpp"
#include "table.hpp"
#include "tag.hpp"

namespace Chimera {
    using ObjectID = HaloID;
    using PlayerID = HaloID;

    struct BaseDynamicObject;

    /**
     * Erase an object
     * @param object_id object to delete
     */
    void delete_object(ObjectID object_id) noexcept;

    /**
     * This represents and points to an object in loopobjects.
     */
    struct ObjectTableIndexHeader {
        /** Object ID index */
        std::uint16_t id;

        PAD(0x4);

        /** Address of the object in loopobjects */
        BaseDynamicObject *object;
    };
    static_assert(sizeof(ObjectTableIndexHeader) == 0xC);

    struct ObjectTable : GenericTable<ObjectTableIndexHeader> {
        /**
         * Get the object table
         * @return object table
         */
        static ObjectTable &get_object_table() noexcept;

        /**
         * Get the object by its ID, returning nullptr if the ID is invalid.
         * @param  object_id This is the ID of the object.
         * @return           Return a pointer to the object or nullptr if the ID is invalid.
         */
        BaseDynamicObject *get_dynamic_object(const ObjectID &object_id) noexcept;

        /**
         * Get the object by an index, returning nullptr if the index is invalid.
         * @param  index This is the index of the object.
         * @return       Return a pointer to the object or nullptr if the index is invalid.
         */
        BaseDynamicObject *get_dynamic_object(std::uint32_t index) noexcept;

        /**
         * Delete an object with an object ID.
         * @param  object_id The object ID of the object
         */
        void delete_object(const ObjectID &object_id) noexcept;

        /**
         * Delete an object with an index.
         * @param  object_id The object ID of the object
         */
        void delete_object(std::uint32_t index) noexcept;
    };

    /**
     * Spawn an object with an tag id.
     * @param  tag_id   Tag ID of the object.
     * @param  x        Object X offset.
     * @param  y        Object Y offset.
     * @param  z        Object Z offset.
     * @param  parent   Parent object.
     * @return          ID of the spawned object.
     */ 
    ObjectID spawn_object(const TagID &tag_id, float x, float y, float z, const ObjectID &parent) noexcept;

    /** This is the type of object an object is. */
    enum ObjectType : std::uint16_t {
        /** Object -> Unit -> Biped */
        OBJECT_TYPE_BIPED = 0,

        /** Object -> Unit -> Vehicle */
        OBJECT_TYPE_VEHICLE,

        /** Object -> Item -> Weapon */
        OBJECT_TYPE_WEAPON,

        /** Object -> Item -> Equipment */
        OBJECT_TYPE_EQUIPMENT,

        /** Object -> Item -> Garbage */
        OBJECT_TYPE_GARBAGE,

        /** Object -> Projectile */
        OBJECT_TYPE_PROJECTILE,

        /** Object -> Scenery */
        OBJECT_TYPE_SCENERY,

        /** Object -> Device -> Device Machine */
        OBJECT_TYPE_DEVICE_MACHINE,

        /** Object -> Device -> Device Control */
        OBJECT_TYPE_DEVICE_CONTROL,

        /** Object -> Device -> Device Light Fixture */
        OBJECT_TYPE_DEVICE_LIGHT_FIXTURE,

        /** Object -> Placeholder */
        OBJECT_TYPE_PLACEHOLDER,

        /** Object -> Sound Scenery */
        OBJECT_TYPE_SOUND_SCENERY
    };

    /** A model node is a part of a model which can have its own position, rotation, and scale. */
    struct ModelNode {
        /** Scale of this part of the model */
        float scale;

        /** Rotation of the model node */
        RotationMatrix rotation;

        /** Position of the model node relative to the world */
        Point3D position;
    };

    /** As of Halo 1.10, 64 nodes is the maximum count. */
    #define MAX_NODES 64

    /**
     * These are objects that are present in an instance of Halo rather than in tag data and have parameters such as location and health.
     */
    struct BaseDynamicObject {
        /** This is the tag ID of the object. */
        TagID tag_id;

        PAD(0x4);
        PAD_BIT(std::uint32_t, 8);

        /** If true, force baseline update. */
        std::uint32_t should_force_baseline_update : 1;

        PAD_BIT(std::uint32_t, 23);

        /** This is the number of ticks the object has existed. */
        TickCount existence_time;

        /** No collision - this is used for backpack weapons. */
        std::uint32_t no_collision : 1;

        /** On the ground */
        std::uint32_t on_ground : 1;

        /** Ignore gravity */
        std::uint32_t ignore_gravity : 1;

        PAD_BIT(std::uint32_t, 2);

        /** Not moving */
        std::uint32_t stationary : 1;

        PAD_BIT(std::uint32_t, 1);

        /** No collision? */
        std::uint32_t no_collision2 : 1;

        PAD_BIT(std::uint32_t, 10);

        /** Cast a shadow if this is off. This is usually off for units, but it can be forced off for other objects. */
        std::uint32_t no_shadow : 1;

        PAD_BIT(std::uint32_t, 2);

        /** Object is in the void. */
        std::uint32_t outside_of_map : 1;

        PAD_BIT(std::uint32_t, 10);
        PAD(0x48);

        /** Coordinates of the object relative to the world or to its parent */
        Point3D position;

        /** Velocity in world units per tick */
        Point3D velocity;

        /** Orientation/rotation */
        Point3D orientation[2];

        /** Rotational velocity of the object in world units per tick */
        Euler3DPYR rotation_velocity;

        /** Unknown */
        std::uint32_t location_id;

        PAD(0x4);

        /** Position of the object's center used for things such as lens flares, triggers, teleporters, etc. This is always relative to the world. */
        Point3D center_position;

        PAD(0x4);

        /** Scale of the object */
        float scale;

        /** Type of object */
        ObjectType type;

        PAD(0x2);
        PAD(0x14);

        /** Tag ID of the animation tag */
        std::uint32_t animation_tag_id;

        /** Current animation index */
        std::uint16_t animation;

        /** Frame of the current animation */
        std::uint16_t animation_frame;

        PAD(0x4);

        /** Base health */
        float base_health;

        /** Base shield */
        float base_shield;

        /** Ratio of health remaining */
        float health;

        /** Ratio of shield remaining */
        float shield;

        /** Current shield damage? */
        float current_shield_damage;

        /** Current health damage? */
        float current_health_damage;

        PAD(0x4);

        /** Recent shield damage taken? */
        float recent_shield_damage;

        /** Recent health damage taken? */
        float recent_health_damage;

        /** Amount of time since shield damage was taken? */
        std::uint32_t recent_shield_damage_time;

        /** Amount of time since health damage was taken? */
        std::uint32_t recent_health_damage_time;

        /** Time in ticks before shields recharge */
        std::uint16_t shield_stun_time;

        PAD_BIT(std::uint16_t, 11);

        /** Immune to damage and damage side effects */
        std::uint16_t invulnerable : 1;

        /** Shields are recharging */
        std::uint16_t shield_recharging : 1;

        PAD_BIT(std::uint16_t, 3);

        PAD(0x10);

        /** Current object ID of the object's current weapon; this may also be the object ID of the object's grenade if throwing */
        ObjectID weapon;

        /** Object ID of the object's parent; if set, then the object's position is relative to the parent */
        ObjectID parent;

        /** Unknown */
        std::uint16_t parent_seat_index;

        PAD(0x2);
        PAD(0x64);

        /** Colors used for things like armor color */
        ColorRGB color_change[4];

        /** Colors used for things like armor color */
        ColorRGB color_change_2[4];

        PAD(0xC);

        /**
         * Get the full object ID of the object.
         * @return full object ID of the object or a null ID if not available
         */
        ObjectID full_object_id() noexcept {
            auto &table = ObjectTable::get_object_table();

            ObjectID returned_id;
            returned_id.whole_id = 0xFFFFFFFF;

            for(std::size_t i = 0; i < table.current_size; i++) {
                auto &object = table.first_element[i];
                if(object.object == this) {
                    returned_id.whole_id = i + 0x10000 * object.id;
                    return returned_id;
                }
            }

            return returned_id;
        }


        /**
         * Get the object's model nodes or nullptr if this is unknown for this object type.
         * @return a pointer to the object model nodes or nullptr if unknown or not available
         */
        ModelNode *nodes() noexcept {
            static const std::size_t model_node_offset[] = {
                0x550,
                0x5C0,
                0x340,
                0x294,
                0x244,
                0x2B0,
                0x1F8,
                0x228,
                0x21C
            };

            if(this->type >= sizeof(model_node_offset) / sizeof(model_node_offset[0])) {
                return nullptr;
            }

            return reinterpret_cast<ModelNode *>(reinterpret_cast<std::byte *>(this) + model_node_offset[this->type]);
        }
    };
    static_assert(sizeof(BaseDynamicObject) == 0x1F4);

    struct UnitRecentDamager {
        /** Last tick this object damaged this unit */
        TickCount last_damage_time;

        /** Total damage done by this object */
        float total_damage;

        /** ObjectID of this object */
        ObjectID object;

        /** Owning player ID of this object */
        PlayerID player;
    };
    static_assert(sizeof(UnitRecentDamager) == 0x10);

    struct UnitDynamicObject : BaseDynamicObject {
        PAD(0x10);

        PAD_BIT(std::uint32_t, 4);

        /** Invisible (camo) */
        std::uint32_t invisible : 1;

        PAD_BIT(std::uint32_t, 14);

        /** Flashlight is on */
        std::uint32_t flashlight : 1;

        /** Do not drop weapons on death */
        std::uint32_t does_not_drop_items : 1;

        PAD_BIT(std::uint32_t, 3);

        /** Frozen. burr */
        std::uint32_t suspended : 1;

        PAD_BIT(std::uint32_t, 7);

        /** Crouching button is pressed */
        std::uint16_t crouch_button : 1;

        /** Jumping button is pressed */
        std::uint16_t jump_button : 1;

        PAD_BIT(std::uint16_t, 2);

        /** Flashlight button is pressed */
        std::uint16_t flashlight_button : 1;

        PAD_BIT(std::uint16_t, 1);

        /** Action button is pressed */
        std::uint16_t action_button : 1;

        /** Melee button is pressed */
        std::uint16_t melee_button : 1;

        PAD_BIT(std::uint16_t, 2);

        /** Is there a meaning of our existence if we only live and die seemingly with no purpose? */
        std::uint16_t reload_button : 1;

        /** Primary fire button is pressed */
        std::uint16_t primary_fire_button : 1;

        /** Secondary fire button is pressed */
        std::uint16_t secondary_fire_button : 1;

        /** Grenade button is pressed */
        std::uint16_t grenade_button : 1;

        /** Exchange weapon button or action button is pressed */
        std::uint16_t exchange_weapon_button : 1;

        PAD_BIT(std::uint16_t, 1);

        PAD(0x2);
        PAD(0xC);

        /** ID of the player owner */
        PlayerID player_id;

        PAD(0x4);

        /** Last tick the unit fired a round */
        TickCount last_bullet_time;

        /** Direction the unit is facing? */
        Point3D facing;

        /** Where the unit wants to aim? */
        Point3D desired_aim;

        /** The aim? */
        Point3D aim;

        /** What is this? */
        Point3D aim_velocity;

        /** More aim stuff? What? */
        Point3D aim2;

        /** Is this it? */
        Point3D aim3;

        PAD(0xC);

        /** Analog running, from -1 (full speed backward) to 1 (full speed upward). */
        float run;

        /** Analog strafing, from -1 (full speed to the right) to 1 (full speed to the left) */
        float strafe;

        /** Analog ascension, from -1 (full speed downward) to 1 (full speed upward). */
        float ascend;

        /** Apparently 1 when firing? */
        float shooting;

        PAD(0xC);

        /** Object ID of the thrown grenade */
        ObjectID thrown_grenade_id;

        PAD(0x8);

        /** 4 when standing, 3 when crouching, and 0 when in a vehicle */
        std::uint8_t crouch_stand_thing;

        /** Current weapon slot */
        std::uint8_t weapon_slot2;

        /** Current weapon type? */
        std::uint8_t weapon_type;

        /** Current animation state of the weapon */
        std::uint8_t animation_state;

        /** 5 when reloading and 7 when meleeing? */
        std::uint8_t reload_melee;

        /** Firing */
        std::uint8_t shooting2;

        /** Animation state of the player's weapon again? */
        std::uint8_t animation_state2;

        /** Crouching... again */
        std::uint8_t crouch2;

        PAD(0x10);

        /** Top-most aim possible? */
        float aim_rectangle_top_x;

        /** Bottom-most aim possible? */
        float aim_rectangle_bottom_x;

        /** Left-most aim possible? */
        float aim_rectangle_left_y;

        /** Right-most aim possible? */
        float aim_rectangle_right_y;

        /** Top-most aim possible again? */
        float look_rectangle_top_x;

        /** Bottom-most aim possible again? */
        float look_rectangle_bottom_x;

        /** Left-most aim possible again? */
        float look_rectangle_left_y;

        /** Right-most aim possible again? */
        float look_rectangle_right_y;

        PAD(0x18);

        /** Current seat (0xFFFF if no seat) */
        std::uint16_t vehicle_seat;

        /** Current weapon slot */
        std::uint16_t weapon_slot;

        /** Weapon slot being changed to */
        std::uint16_t next_weapon_slot;

        PAD(0x2);

        /** Weapons the unit owns in slot order */
        ObjectID weapons[4];

        /** Last tick a weapon was used */
        TickCount weapon_last_use[4];

        PAD(0x4);

        /** Grenade type (0 = fragmentation; 1 = plasma) */
        std::uint8_t current_grenade_type;

        /** Next grenade type */
        std::uint8_t next_grenade_type;

        /** Primary grenade count */
        std::uint8_t primary_grenade_count;

        /** Secondary grenades count */
        std::uint8_t secondary_grenade_count;

        /** Zoom level */
        std::uint8_t zoom_level;

        /** Zoom level being changed to */
        std::uint8_t desired_zoom_level;

        PAD(2);

        /** Object ID of the controller of this unit */
        ObjectID controller;

        /** Object ID of the gunner of this unit */
        ObjectID gunner;

        PAD(0x14);

        /** Intensity of the flashlight */
        float flashlight_intensity;

        /** Remaining battery life of the flashlight */
        float flashlight_battery;

        /* Intensity of the night vision */
        float night_vision_intensity;

        PAD(0x30);

        /* Invisibility of the unit (1 = full camo; 0 = no camo) */
        float invisibility;

        PAD(0xB0);

        /** Most recent damaging objects */
        UnitRecentDamager recent_damager[4];

        PAD(0x5C);
    };
    static_assert(sizeof(UnitDynamicObject) == 0x4CC);

    struct WeaponTriggerState {
        PAD(0x1);

        /** Unknown */
        std::uint8_t trigger_state;

        /** Unknown */
        std::uint8_t trigger_time;

        PAD(1);

        /** Unknown */
        std::uint32_t not_firing;

        /** Unknown */
        std::uint32_t auto_reload;

        PAD(0x2);

        /** Number of rounds since the last tracer */
        std::uint16_t rounds_since_last_tracer;

        /** Rare of fire */
        float rate_of_fire;

        /** Ejection port recovery time */
        float ejection_port_recovery_time;

        /** Illumination recovery time */
        float illumination_recovery_time;

        /** Unknown */
        float error_angle_thingy;

        PAD(0x8);
    };
    static_assert(sizeof(WeaponTriggerState) == 0x28);

    struct WeaponMagazineState {
        /** Unknown */
        std::uint16_t magazine_state;

        /** Unknown */
        std::uint16_t chambering_time;

        PAD(0x2);

        /** Total unloaded ammo */
        std::uint16_t ammo;

        /** Total loaded ammo */
        std::uint16_t loaded_ammo;

        PAD(0x2);

        PAD(0x4);
    };
    static_assert(sizeof(WeaponMagazineState) == 0x10);

    struct WeaponDynamicObject : BaseDynamicObject {
        PAD(0x40);

        /** Unknown */
        float primary_trigger;

        PAD(0x4);

        /** Heat */
        float heat;

        /** Age; 1 = unusable */
        float age;

        /** Illumination */
        float illumination;

        /** Unknown */
        float light_power;

        PAD(0x14);

        /** Trigger states */
        WeaponTriggerState triggers[2];

        /** Magazine states */
        WeaponMagazineState magazines[2];

    };
    static_assert(sizeof(WeaponDynamicObject) == 0x2D0);
}

#endif
