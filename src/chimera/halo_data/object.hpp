// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_OBJECT_HPP
#define CHIMERA_OBJECT_HPP

#include <cstddef>
#include <cstdint>

#include "../math_trig/math_trig.hpp"
#include "pad.hpp"
#include "table.hpp"
#include "tag.hpp"
#include "ai_defs.hpp"

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

    enum {
        NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS = 4,
        NUMBER_OF_INCOMING_OBJECT_FUNCTIONS = 4,
        MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT = 8,
        MAXIMUM_REGIONS_PER_OBJECT = 8
    };

    enum ObjectType : std::uint16_t {
        OBJECT_TYPE_BIPED,
        OBJECT_TYPE_VEHICLE,
        OBJECT_TYPE_WEAPON,
        OBJECT_TYPE_EQUIPMENT,
        OBJECT_TYPE_GARBAGE,
        OBJECT_TYPE_PROJECTILE,
        OBJECT_TYPE_SCENERY,
        OBJECT_TYPE_DEVICE_MACHINE,
        OBJECT_TYPE_DEVICE_CONTROL,
        OBJECT_TYPE_DEVICE_LIGHT_FIXTURE,
        OBJECT_TYPE_PLACEHOLDER,
        OBJECT_TYPE_SOUND_SCENERY,
        NUMBER_OF_OBJECT_TYPES
    };

    enum ObjectChangeColor : std::uint16_t {
        OBJECT_CHANGE_COLOR_A,
        OBJECT_CHANGE_COLOR_B,
        OBJECT_CHANGE_COLOR_C,
        OBJECT_CHANGE_COLOR_D,
        NUMBER_OF_OBJECT_CHANGE_COLORS
    };

    enum ObjectDataFlags {
        OBJECT_DATA_FLAGS_INVISIBLE_BIT,
        OBJECT_DATA_FLAGS_ON_GROUND_BIT,
        OBJECT_DATA_FLAGS_ON_MEDIA_BIT,
        OBJECT_DATA_FLAGS_PARTIALLY_UNDER_MEDIA_BIT,
        OBJECT_DATA_FLAGS_WHOLLY_UNDER_MEDIA_BIT,
        OBJECT_DATA_FLAGS_AT_REST_BIT,
        OBJECT_DATA_FLAGS_CHILD_LOCATIONS_INVALID_BIT,
        OBJECT_DATA_FLAGS_ANIMATES_AUTOMATICALLY_BIT,
        OBJECT_DATA_FLAGS_HAS_ATTACHED_LIGHTS_BIT,
        OBJECT_DATA_FLAGS_HAS_ATTACHED_SHADER_BIT,
        OBJECT_DATA_FLAGS_HAS_ATTACHED_LOOPING_SOUNDS_BIT,
        OBJECT_DATA_FLAGS_CONNECTED_TO_MAP_BIT,
        OBJECT_DATA_FLAGS_MIRRORED_BIT,
        OBJECT_DATA_FLAGS_DYNAMIC_LIGHTING_RECOMPUTE_BIT,
        OBJECT_DATA_FLAGS_STATIC_LIGHTING_RECOMPUTE_BIT,
        OBJECT_DATA_FLAGS_STATIC_LIGHTING_RAYCAST_SIDEWAYS_BIT,
        OBJECT_DATA_FLAGS_GARBAGE_BIT,
        OBJECT_DATA_FLAGS_CANNOT_BE_GARBAGE_BIT,
        OBJECT_DATA_FLAGS_SHADOWLESS_BIT,
        OBJECT_DATA_FLAGS_DELETED_WHEN_DEACTIVATED_BIT,
        OBJECT_DATA_FLAGS_CANNOT_BE_ACTIVATED_BIT,
        OBJECT_DATA_FLAGS_OUTSIDE_OF_MAP_BIT,
        OBJECT_DATA_FLAGS_MOVIE_STAR_BIT,
        OBJECT_DATA_FLAGS_DO_NOT_RECOMPUTE_NODE_MATRICES_BIT,
        OBJECT_DATA_FLAGS_NO_COLLISIONS_BIT,
        OBJECT_DATA_FLAGS_HAS_COLLISION_MODEL_BIT,
        OBJECT_DATA_FLAGS_FORCE_BASELINE_UPDATE_BIT,
        OBJECT_DATA_FLAGS_HAS_BEEN_UPDATED_FROM_NETWORK_BIT,
        NUMBER_OF_OBJECT_DATA_FLAGS
    };

    enum ObjectDamageFlags {
        OBJECT_DAMAGE_FLAGS_PASSED_BODY_DAMAGE_THRESHOLD_BIT,
        OBJECT_DAMAGE_FLAGS_PASSED_SHIELD_DAMAGE_THRESHOLD_BIT,
        OBJECT_DAMAGE_FLAGS_DEAD_BIT,
        OBJECT_DAMAGE_FLAGS_SHIELD_DEPLETED_BIT,
        OBJECT_DAMAGE_FLAGS_SHIELD_OVER_CHARGING_BIT,
        OBJECT_DAMAGE_FLAGS_DIE_ACT_OF_GOD_BIT,
        OBJECT_DAMAGE_FLAGS_DIE_ACT_OF_GOD_SILENT_BIT,
        OBJECT_DAMAGE_FLAGS_MELEE_ATTACK_INHIBITED_BIT,
        OBJECT_DAMAGE_FLAGS_RANGED_ATTACK_INHIBITED_BIT,
        OBJECT_DAMAGE_FLAGS_WALKING_INHIBITED_BIT,
        OBJECT_DAMAGE_FLAGS_CANNOT_HOLD_WEAPON_BIT,
        OBJECT_DAMAGE_FLAGS_CANNOT_TAKE_DAMAGE_BIT,
        OBJECT_DAMAGE_FLAGS_SHIELD_CHARGING_BIT,
        OBJECT_DAMAGE_FLAGS_DIE_ACT_OF_GOD_NO_STATISTICS_BIT,
        NUMBER_OF_OBJECT_DAMAGE_FLAGS
    };

    /** Scenario location */
    struct Location {
        std::uint32_t leaf_index;
        std::int16_t cluster_index;
        std::uint16_t bonus;
    };
    static_assert(sizeof(Location) == 0x8);

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

    struct BaseObjectNetwork {
        bool valid_position;
        Vector3D position;
        bool valid_forward_and_up;
        Vector3D forward;
        Vector3D up;
        bool valid_angular_velocity;
        Vector3D angular_velocity;
        bool valid_timestamp;
        TickCount timestamp;
    };
    static_assert(sizeof(BaseObjectNetwork) == 0x44);

    struct AnimationState {
        std::int16_t index;
        std::int16_t frame_index;
    };
    static_assert(sizeof(AnimationState) == 0x4);

    struct ObjectAnimationState {
        TagID animation_tag_id;
        AnimationState state;
        std::int16_t interpolation_frame_index;
        std::int16_t interpolation_frame_count;
    };
    static_assert(sizeof(ObjectAnimationState) == 0xC);

    struct ObjectHeaderBlockReference {
        std::uint16_t size;
        std::uint16_t offset;
    };
    static_assert(sizeof(ObjectHeaderBlockReference) == 0x4);

    struct ObjectDatum {
        std::uint32_t network_role;
        std::uint32_t unknown_flags;
        TickCount existence_time;
        std::uint32_t flags;
        std::int32_t magic_number;
        BaseObjectNetwork network;
        Point3D position;

        /** Velocity in world units per tick */
        Vector3D translational_velocity;

        /** Orientation/rotation */
        Vector3D forward;
        Vector3D up;

        /** Rotational velocity of the object in world units per tick */
        Vector3D angular_velocity;

        Location location;
        Point3D bounding_sphere_center;
        float bounding_sphere_radius;
        float scale;

        /** Type of object */
        ObjectType type;

        std::uint16_t render_flags;
        std::int16_t owner_team_index;
        std::int16_t name_index;
        std::int16_t idle_ticks;
        std::int16_t variant_number;
        std::int32_t owner_player_index;
        std::int32_t owner_object_index;
        std::int32_t owner_object_definition_index;
        ObjectAnimationState animation;

        /** Base health */
        float maximum_body_vitality;

        /** Base shield */
        float maximum_shield_vitality;

        /** Ratio of health remaining */
        float body_vitality;

        /** Ratio of shield remaining */
        float shield_vitality;

        /** Current damage */
        float current_shield_damage;
        float current_body_damage;

        /** shield of this object gets damaged first */
        ObjectID umbrella_shield_object_index;

        float recent_shield_damage;
        float recent_body_damage;
        std::int32_t shield_damage_decay_timer;
        std::int32_t body_damage_decay_timer;

        /** Time in ticks before shields recharge */
        std::uint16_t shield_stun_ticks;

        std::uint16_t damage_flags;
        std::uint32_t unused_for_bernie;
        std::int32_t first_cluster_reference_index;
        ObjectID next_garbage_object_index;
        ObjectID next_object_index;

        /** Current object ID of the object's current weapon; this may also be the object ID of the object's grenade if throwing */
        ObjectID first_child_object_index;

        /** Object ID of the object's parent; if set, then the object's position is relative to the parent */
        ObjectID parent_object_index;

        std::int8_t parent_node_index;
        std::uint8_t pad[2];
        std::uint8_t functions_active_flags;
        float incoming_function_values[NUMBER_OF_INCOMING_OBJECT_FUNCTIONS];
        float outgoing_function_values[NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS];
        std::int8_t attachment_types[MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT];
        std::int32_t attachment_indices[MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT];
        std::int32_t first_widget_index;
        std::int32_t cached_render_state_index;
        std::uint16_t regions_destroyed_flags;
        std::int16_t forced_shader_permutation_index;
        std::uint8_t region_damage[MAXIMUM_REGIONS_PER_OBJECT];
        std::uint8_t region_permutations[MAXIMUM_REGIONS_PER_OBJECT];

        /** Colors used for things like armor color */
        ColorRGB base_change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];
        ColorRGB outgoing_change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];

        ObjectHeaderBlockReference original_node_orientations;
        ObjectHeaderBlockReference node_orientations;
        ObjectHeaderBlockReference node_matrices;
    };
    static_assert(sizeof(ObjectDatum) == 0x1F0);

    /**
     * These are objects that are present in an instance of Halo rather than in tag data and have parameters such as location and health.
     */
    struct BaseDynamicObject {
        TagID definition_index;
        ObjectDatum object;

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

            if(this->object.type >= sizeof(model_node_offset) / sizeof(model_node_offset[0])) {
                return nullptr;
            }

            return reinterpret_cast<ModelNode *>(reinterpret_cast<std::byte *>(this) + model_node_offset[this->object.type]);
        }
    };
    static_assert(sizeof(BaseDynamicObject) == 0x1F4);

    enum {
        MAXIMUM_WEAPONS_PER_UNIT = 4,
        NUMBER_OF_UNIT_USER_ANIMATIONS = 2,
        MAXIMUM_ATTACKERS_PER_UNIT = 4
    };

    enum UnitDatumFlags {
        UNIT_DATUM_FLAGS_ACTIVELY_CONTROLLED_BIT,
        UNIT_DATUM_FLAGS_CHANGED_AIMING_ANGLES_BIT,
        UNIT_DATUM_FLAGS_DESIRED_WEAPON_INVALID_BIT,
        UNIT_DATUM_FLAGS_EXACT_FACING_BIT,
        UNIT_DATUM_FLAGS_ACTIVE_CAMOUFLAGED_BIT,
        UNIT_DATUM_FLAGS_SUPER_CAMOUFLAGED_BIT,
        UNIT_DATUM_FLAGS_CONTROLLABLE_BIT,
        UNIT_DATUM_FLAGS_IGNORE_HARD_PINGS_BIT,
        UNIT_DATUM_FLAGS_MUST_SET_UP_DIALOGUE_BIT,
        UNIT_DATUM_FLAGS_PLACED_HERE_DEAD_BIT,
        UNIT_DATUM_FLAGS_IGNORED_BY_ACTORS_BIT,
        UNIT_DATUM_FLAGS_PREFERRED_TARGET_BIT,
        UNIT_DATUM_FLAGS_NO_FALLING_DAMAGE_BIT,
        UNIT_DATUM_FLAGS_FEIGN_DEATH_ALLOWED_BIT,
        UNIT_DATUM_FLAGS_AIM_WITHOUT_TURNING_BIT,
        UNIT_DATUM_FLAGS_ATTACHED_MELEE_ATTACK_BIT,
        UNIT_DATUM_FLAGS_NOT_ENTERABLE_BY_PLAYER_BIT,
        UNIT_DATUM_FLAGS_SPAWNED_ACTORS_BIT,
        UNIT_DATUM_FLAGS_UNLOADING_BIT,
        UNIT_DATUM_FLAGS_INTEGRATED_LIGHT_ON_BIT,
        UNIT_DATUM_FLAGS_DOESNT_DROP_ITEMS_BIT,
        UNIT_DATUM_FLAGS_HAS_ALREADY_BEEN_HACKY_HACKY_HACKED_BIT,
        UNIT_DATUM_FLAGS_CANNOT_BLINK_BIT,
        UNIT_DATUM_FLAGS_IMPERVIOUS_BIT,
        UNIT_DATUM_FLAGS_SUSPENDED_BIT,
        UNIT_DATUM_FLAGS_RUNNING_BLINDLY_BIT,
        UNIT_DATUM_FLAGS_INTEGRATED_NIGHT_VISION_ON_BIT,
        UNIT_DATUM_FLAGS_POSSESSED_BY_RECORDING_BIT,
        UNIT_DATUM_FLAGS_DESIRED_INTEGRATED_LIGHT_ON_BIT,
        UNIT_DATUM_FLAGS_DESIRED_INTEGRATED_LIGHT_OFF_BIT,
        NUMBER_OF_UNIT_DATUM_FLAGS
    };

    enum UnitControlFlags {
        UNIT_CONTROL_FLAGS_CROUCH_MODIFIER_BIT,
        UNIT_CONTROL_FLAGS_JUMP_BIT,
        UNIT_CONTROL_FLAGS_USER_ANIMATION1_BIT,
        UNIT_CONTROL_FLAGS_USER_ANIMATION2_BIT,
        UNIT_CONTROL_FLAGS_INTEGRATED_LIGHT_BIT,
        UNIT_CONTROL_FLAGS_EXACT_FACING_BIT,
        UNIT_CONTROL_FLAGS_ACTION_BIT,
        UNIT_CONTROL_FLAGS_USE_EQUIPMENT_BIT,
        UNIT_CONTROL_FLAGS_LOOK_DONT_TURN_BIT,
        UNIT_CONTROL_FLAGS_FORCE_ALERT_BIT,
        UNIT_CONTROL_FLAGS_WEAPON_RELOAD_BIT,
        UNIT_CONTROL_FLAGS_WEAPON_PRIMARY_TRIGGER_BIT,
        UNIT_CONTROL_FLAGS_WEAPON_SECONDARY_TRIGGER_BIT,
        UNIT_CONTROL_FLAGS_THROW_GRENADE_BIT,
        UNIT_CONTROL_FLAGS_SWAP_WEAPONS_BIT,
        NUMBER_OF_UNIT_CONTROL_FLAGS,
        UNIT_CONTROL_DRIVER_MASK = 63,
        UNIT_CONTROL_GUNNER_MASK = 31744
    };

    enum UnitGrenadeType : std::int8_t {
        UNIT_GRENADE_TYPE_HUMAN_FRAGMENTATION,
        UNIT_GRENADE_TYPE_COVENANT_PLASMA,
        NUMBER_OF_UNIT_GRENADE_TYPES
    };

    enum {
        UNIT_POWERED_SEAT_DRIVER,
        UNIT_POWERED_SEAT_GUNNER,
        NUMBER_OF_UNIT_POWERED_SEATS
    };

    struct UnitAnimation {
        std::uint16_t flags;
        std::int16_t aiming_screen_index;
        std::int16_t looking_screen_index;
        std::int16_t last_ping_animation_index;
        std::int8_t seat_index;
        std::int8_t weapon_index;
        std::int8_t weapon_type_index;
        std::int8_t state;
        std::int8_t action;
        std::int8_t overlay_action;
        std::int8_t desired_state;
        std::int8_t base_seat_index;
        std::int8_t emotion_index;
        AnimationState action_animation;
        AnimationState overlay_action_animation;
        AnimationState soft_ping_animation;
        bool aiming_with_euler_screen;
        bool looking_with_euler_screen;
        Bounds2D aiming_screen_bounds;
        Bounds2D looking_screen_bounds;
        std::int32_t external_animation_graph_index;
        AnimationState external_animation;
    };
    static_assert(sizeof(UnitAnimation) == 0x48);

    struct UnitSpeechItem {
        std::int16_t priority;
        std::int16_t vocalization_type;
        std::int32_t sound_definition_index;
        std::int16_t delay_time;
        std::int16_t ai_notification_delay;
        std::int16_t pause_time;
        std::uint16_t pad;
        AIInformationPacket ai;
    };
    static_assert(sizeof(UnitSpeechItem) == 0x30);

    struct UnitSpeech {
        UnitSpeechItem current;
        UnitSpeechItem queued;
        std::int16_t damage_minor_decay_timer;
        std::int16_t damage_minor_sounds;
        std::int16_t damage_minor_timer;
        std::int16_t damage_major_timer;
        std::int32_t last_speech_finished_time;
        bool played;
        bool notified_ai;
        bool finished;
        std::uint8_t pad;
        std::int16_t pre_delay_timer;
        std::int16_t sound_timer;
        std::int16_t ai_delay_timer;
        std::int16_t post_delay_timer;
        std::int32_t impulse_sound_index;
    };
    static_assert(sizeof(UnitSpeech) == 0x7C);

    struct UnitAttacker {
        /** Last tick this object damaged this unit */
        TickCount game_time_stamp;

        /** Total damage done by this object */
        float damage_inflicted;

        /** ObjectID of this object */
        ObjectID object_index;

        /** Owning player ID of this object */
        PlayerID player_index;
    };
    static_assert(sizeof(UnitAttacker) == 0x10);

    struct UnitControlData {
        std::int8_t animation_state;
        std::int8_t aiming_speed;
        std::uint16_t control_flags;
        std::int16_t weapon_index;
        std::int16_t grenade_index;
        std::int16_t zoom_level;
        std::uint16_t pad;
        Vector3D throttle;
        float primary_trigger;
        Vector3D facing_vector;
        Vector3D aiming_vector;
        Vector3D looking_vector;
    };
    static_assert(sizeof(UnitControlData) == 0x40);

    struct UnitDatum {
        std::int32_t actor_index;
        std::int32_t swarm_actor_index;
        std::int32_t swarm_next_unit_index;
        std::int32_t swarm_prev_unit_index;
        std::uint32_t flags;
        std::uint32_t control_flags;
        std::int16_t timer;
        std::int8_t shield_sap_timeout;
        std::int8_t magic_seat_index;
        std::int32_t persistent_control_timer;
        std::uint32_t persistent_control_flags;

        /** ID of the player owner */
        PlayerID player_index;

        std::int16_t last_unit_effect_type;
        std::uint16_t override_emotion_animation_index;

        /** Last tick the unit fired a round */
        std::int32_t game_time_at_last_unit_effect;

        Vector3D desired_facing_vector;
        Vector3D desired_aiming_vector;
        Vector3D aiming_vector;
        Vector3D aiming_velocity;
        Vector3D desired_looking_vector;
        Vector3D looking_vector;
        Vector3D looking_velocity;

        /** Analog 0 - 1 movement */
        Vector3D throttle;

        /** 1.0f when firing, 0.0f when not */
        float primary_trigger;

        std::int8_t aiming_speed;
        std::int8_t melee_attack_state;
        std::int8_t melee_continuous_damage_effect_timer;
        std::uint8_t flaming_death_delay;
        std::int8_t weapon_drop_delay_ticks;
        std::int8_t grenade_throw_state;
        std::int16_t grenade_throw_ticks;
        std::int16_t grenade_throw_full_power_ticks;

        /** Object ID of the thrown grenade */
        ObjectID grenade_object_index;

        UnitAnimation animation;
        float ambient_illumination;
        float self_illumination;
        float mouth_aperture;
        std::int32_t last_entrance_attempt;

        /** Current seat (0xFFFF if no seat) */
        std::uint16_t parent_seat_index;

        std::uint16_t current_weapon_index;
        std::uint16_t desired_weapon_index;
        std::uint16_t pad;

        /** Weapons the unit owns in slot order */
        ObjectID weapon_object_indices[MAXIMUM_WEAPONS_PER_UNIT];

        /** Last tick a weapon was used */
        TickCount weapon_last_used_at_game_time[MAXIMUM_WEAPONS_PER_UNIT];

        ObjectID equipment_object_index;
        UnitGrenadeType current_grenade_index;
        UnitGrenadeType desired_grenade_index;
        std::int8_t grenade_counts[NUMBER_OF_UNIT_GRENADE_TYPES];
        std::uint8_t current_zoom_level;
        std::uint8_t desired_zoom_level;
        std::int8_t gunner_inactive_ticks;
        std::uint8_t aiming_change;
        ObjectID driver_object_index;
        ObjectID gunner_object_index;
        ObjectID last_vehicle_index;
        std::int32_t game_time_at_last_vehicle_exit;
        std::int16_t fake_encounter_index;
        std::int16_t fake_squad_index;
        float seat_power[NUMBER_OF_UNIT_POWERED_SEATS];
        float integrated_light_power;
        float integrated_light_battery;
        float integrated_night_vision_power;
        Point3D seat_last_position;
        Vector3D seat_last_velocity;
        Vector3D seat_acceleration;
        Vector3D seat_desired_acceleration;

        /** Invisibility of the unit (1 = full camo; 0 = no camo) */
        float active_camouflage;

        float active_camouflage_super_amount;
        std::int32_t dialogue_index;
        UnitSpeech speech;
        std::int16_t last_damage_category;
        std::int16_t delayed_damage_timer;
        float delayed_damage_peak;
        std::int32_t delayed_damage_attacker_object_index;
        std::int32_t flaming_death_attacker_object_index;
        float run_blindly_angle;
        float run_blindly_angle_delta;
        std::int32_t time_of_death;
        std::int16_t feign_death_timer;
        std::int16_t cause_for_camo_regrowth;
        float body_stun;
        std::int16_t body_stun_ticks;
        std::int16_t killing_spree_count;
        std::int32_t killing_spree_last_time;

        /** Most recent damaging objects */
        UnitAttacker arrackers[MAXIMUM_ATTACKERS_PER_UNIT];

        std::int16_t user_animation_indices[NUMBER_OF_UNIT_USER_ANIMATIONS];
        bool force_local_update;
        bool is_from_network_data_valid;
        union {
            UnitControlData most_recent_control_data_for_network;
            UnitControlData most_recent_control_data_from_network;
        };
        bool did_just_complete_client_update;
        std::int32_t last_completed_client_update_id;
        Point3D position_after_completing_last_client_update;
    };
    static_assert(sizeof(UnitDatum) == 0x2D8);

    struct UnitDynamicObject : BaseDynamicObject {
        UnitDatum unit;
    };
    static_assert(sizeof(UnitDynamicObject) == 0x4CC);

    enum ItemDatumFlags {
        ITEM_DATUM_FLAGS_IN_UNIT_INVENTORY_BIT,
        ITEM_DATUM_FLAGS_BELONGS_TO_PLAYER_BIT,
        ITEM_DATUM_FLAGS_HAS_NONZERO_ANGULAR_VELOCITY_BIT,
        ITEM_DATUM_FLAGS_ON_STRUCTURE_BIT,
        ITEM_DATUM_FLAGS_ON_OBJECT_BIT,
        ITEM_DATUM_FLAGS_DOES_NOT_ACCELERATE_BIT,
        NUMBER_OF_ITEM_DATUM_FLAGS
    };

    struct _item_datum {
        std::uint32_t flags;
        std::int16_t detonation_ticks;
        std::int16_t rested_surface_index;
        std::int16_t bsp_index;
        std::uint16_t pad;
        std::int32_t ignore_object_index;
        std::int32_t last_owned_time;
        std::int32_t item_on_rest_object_index;
        Point3D item_rest_object_offset;
        Vector3D rotation_axis;
        float rotation_sine;
        float rotation_cosine;
    };
    static_assert(sizeof(_item_datum) == 0x38);

    struct ItemDynamicObject : BaseDynamicObject {
        _item_datum item;
    };

    enum WeaponMagazineState : std::int16_t {
        WEAPON_MAGAZINE_STATE_IDLE,
        WEAPON_MAGAZINE_STATE_RELOADING,
        WEAPON_MAGAZINE_STATE_UNCHAMBERED,
        WEAPON_MAGAZINE_STATE_CHAMBERING,
        NUMBER_OF_WEAPON_MAGAZINE_STATES
    };

    struct WeaponMagazine {
        WeaponMagazineState state;
        std::int16_t state_timer;
        std::int16_t original_time;
        std::int16_t rounds_total;
        std::int16_t rounds_loaded;
        std::int16_t rounds_fractional_recharged;
    };
    static_assert(sizeof(WeaponMagazine) == 0xC);

    enum WeaponTriggerDatumFlags {
        WEAPON_TRIGGER_DATUM_FLAGS_RELEASED_SINCE_LAST_SHOT_BIT,
        WEAPON_TRIGGER_DATUM_FLAGS_WAS_DOWN_BIT,
        WEAPON_TRIGGER_DATUM_FLAGS_TOGGLED_BIT,
        WEAPON_TRIGGER_DATUM_FLAGS_USELESS_BIT,
        WEAPON_TRIGGER_DATUM_FLAGS_BLURRED_BIT,
        WEAPON_TRIGGER_DATUM_FLAGS_FIRED_BEFORE_CHARGING_BIT,
        NUMBER_OF_WEAPON_TRIGGER_DATUM_FLAGS
    };

    enum WeaponTriggerState : std::int8_t {
        WEAPON_TRIGGER_STATE_IDLE,
        WEAPON_TRIGGER_STATE_OVERLOADING,
        WEAPON_TRIGGER_STATE_CHARGING,
        WEAPON_TRIGGER_STATE_CHARGED,
        WEAPON_TRIGGER_STATE_RECOVERING,
        WEAPON_TRIGGER_STATE_TRACKING,
        WEAPON_TRIGGER_STATE_SPEWING,
        WEAPON_TRIGGER_STATE_LOCKED,
        WEAPON_TRIGGER_STATE_UNINITIALIZED,
        NUMBER_OF_WEAPON_TRIGGER_STATES
    };

    struct WeaponTrigger {
        std::int8_t idle_ticks;
        WeaponTriggerState state;
        std::int16_t state_timer;
        std::uint32_t flags;
        std::uint16_t firing_effects_used_flags;
        std::int16_t firing_effect_index;
        std::int16_t firing_effect_shots_remaining;
        std::int16_t sequential_non_tracer_rounds;
        float rate_of_fire;
        float ejection_port_position;
        float illumination;
        float error;
        std::int32_t charging_effect_index;
        std::int8_t delay_ticks_before_empty_clip_auto_reload;
    };
    static_assert(sizeof(WeaponTrigger) == 0x28);

    struct WeaponDatumNetworkData {
        Point3D position;
        Vector3D translational_velocity;
        Vector3D angular_velocity;
        std::int16_t magazine_0_rounds_total;
        std::int16_t magazine_1_rounds_total;
        float age;
    };
    static_assert(sizeof(WeaponDatumNetworkData) == 0x2C);

    enum WeaponState : std::int8_t {
        WEAPON_STATE_IDLE,
        WEAPON_STATE_PRIMARY_RECOIL,
        WEAPON_STATE_SECONDARY_RECOIL,
        WEAPON_STATE_PRIMARY_CHAMBER,
        WEAPON_STATE_SECONDARY_CHAMBER,
        WEAPON_STATE_PRIMARY_RELOAD,
        WEAPON_STATE_SECONDARY_RELOAD,
        WEAPON_STATE_PRIMARY_CHARGED,
        WEAPON_STATE_SECONDARY_CHARGED,
        WEAPON_STATE_READY,
        WEAPON_STATE_PUT_AWAY,
        NUMBER_OF_WEAPON_STATES
    };

    struct WeaponDatum {
        std::uint32_t flags;
        std::int16_t control_flags;
        float primary_trigger;
        WeaponState state;
        WeaponState last_reported_state;
        std::int16_t state_timer;
        float heat;
        float age;
        float overcharged;
        float integrated_light_power;
        std::int8_t integrated_light_delay_ticks;
        std::int32_t tracked_object_index;
        float recoil_angular_velocity;
        std::int16_t recoil_recovery_time;
        std::int16_t shots_until_demotion;
        std::int16_t alternate_shots_loaded;
        WeaponTrigger triggers[2];
        WeaponMagazine magazines[2];
        AnimationState animation;
        std::int32_t overheated_effect_index;
        std::int32_t game_time_last_fired;
        std::int16_t magazine_rounds_total_at_reload_start[2];
        std::int16_t magazine_rounds_loaded_at_reload_start[2];
        std::int16_t magazine_rounds_total_to_use_at_reload_finish[2];
        bool baseline_valid;
        std::uint8_t baseline_index;
        std::uint8_t message_index;
        WeaponDatumNetworkData baseline;
        bool last_network_data_valid;
        WeaponDatumNetworkData last_network_data;
    };
    static_assert(sizeof(WeaponDatum) == 0x114);

    struct WeaponDynamicObject : ItemDynamicObject {
        WeaponDatum weapon;
    };
    static_assert(sizeof(WeaponDynamicObject) == 0x340);

    struct DeviceDatum {
        std::uint32_t flags;
        std::int16_t power_group_index;
        float power;
        float power_velocity;
        std::int16_t position_group_index;
        float position;
        float position_velocity;
        std::int16_t delay_ticks;
    };
    static_assert(sizeof(DeviceDatum) == 0x20);

    struct DeviceDynamicObject : BaseDynamicObject {
        DeviceDatum device;
    };
    static_assert(sizeof(DeviceDynamicObject) == 0x214);

    struct MachineDatum {
        std::uint32_t flags;
        std::int32_t door_open_ticks;
        Point3D elevator_position;
    };
    static_assert(sizeof(MachineDatum) == 0x14);

    struct MachineDynamicObject : DeviceDynamicObject {
        MachineDatum machine;
    };
    static_assert(sizeof(MachineDynamicObject) == 0x228);
}

#endif
