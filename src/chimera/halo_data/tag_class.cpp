// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "tag_class.hpp"

namespace Chimera {
    TagClassInt tag_class_from_string(const char *tag_class) {
        if(std::strcmp(tag_class, "actor") == 0) 
            return TAG_CLASS_ACTOR;
        else if(std::strcmp(tag_class, "actor_variant") == 0) 
            return TAG_CLASS_ACTOR_VARIANT;
        else if(std::strcmp(tag_class, "antenna") == 0)
            return TAG_CLASS_ANTENNA;
        else if(std::strcmp(tag_class, "model_animations") == 0)
            return TAG_CLASS_MODEL_ANIMATIONS;
        else if(std::strcmp(tag_class, "biped") == 0)
            return TAG_CLASS_BIPED;
        else if(std::strcmp(tag_class, "bitmap") == 0)
            return TAG_CLASS_BITMAP;
        else if(std::strcmp(tag_class, "spheroid") == 0)
            return TAG_CLASS_SPHEROID;
        else if(std::strcmp(tag_class, "continuous_damage_effect") == 0)
            return TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT;
        else if(std::strcmp(tag_class, "model_collision_geometry") == 0)
            return TAG_CLASS_MODEL_COLLISION_GEOMETRY;
        else if(std::strcmp(tag_class, "color_table") == 0)
            return TAG_CLASS_COLOR_TABLE;
        else if(std::strcmp(tag_class, "contrail") == 0)
            return TAG_CLASS_CONTRAIL;
        else if(std::strcmp(tag_class, "device_control") == 0)
            return TAG_CLASS_DEVICE_CONTROL;
        else if(std::strcmp(tag_class, "decal") == 0)
            return TAG_CLASS_DECAL;
        else if(std::strcmp(tag_class, "ui_widget_definition") == 0)
            return TAG_CLASS_UI_WIDGET_DEFINITION;
        else if(std::strcmp(tag_class, "input_device_defaults") == 0)
            return TAG_CLASS_INPUT_DEVICE_DEFAULTS;
        else if(std::strcmp(tag_class, "device") == 0)
            return TAG_CLASS_DEVICE;
        else if(std::strcmp(tag_class, "detail_object_collection") == 0)
            return TAG_CLASS_DETAIL_OBJECT_COLLECTION;
        else if(std::strcmp(tag_class, "effect") == 0)
            return TAG_CLASS_EFFECT;
        else if(std::strcmp(tag_class, "equipment") == 0)
            return TAG_CLASS_EQUIPMENT;
        else if(std::strcmp(tag_class, "flag") == 0)
            return TAG_CLASS_FLAG;
        else if(std::strcmp(tag_class, "fog") == 0)
            return TAG_CLASS_FOG;
        else if(std::strcmp(tag_class, "font") == 0)
            return TAG_CLASS_FONT;
        else if(std::strcmp(tag_class, "lightning") == 0)
            return TAG_CLASS_LIGHTNING;
        else if(std::strcmp(tag_class, "material_effects") == 0)
            return TAG_CLASS_MATERIAL_EFFECTS;
        else if(std::strcmp(tag_class, "garbage") == 0)
            return TAG_CLASS_GARBAGE;
        else if(std::strcmp(tag_class, "glow") == 0)
            return TAG_CLASS_GLOW;
        else if(std::strcmp(tag_class, "grenade_hud_interface") == 0)
            return TAG_CLASS_GRENADE_HUD_INTERFACE;
        else if(std::strcmp(tag_class, "hud_message_text") == 0)
            return TAG_CLASS_HUD_MESSAGE_TEXT;
        else if(std::strcmp(tag_class, "hud_number") == 0)
            return TAG_CLASS_HUD_NUMBER;
        else if(std::strcmp(tag_class, "hud_globals") == 0)
            return TAG_CLASS_HUD_GLOBALS;
        else if(std::strcmp(tag_class, "item") == 0)
            return TAG_CLASS_ITEM;
        else if(std::strcmp(tag_class, "item_collection") == 0)
            return TAG_CLASS_ITEM_COLLECTION;
        else if(std::strcmp(tag_class, "damage_effect") == 0)
            return TAG_CLASS_DAMAGE_EFFECT;
        else if(std::strcmp(tag_class, "lens_flare") == 0)
            return TAG_CLASS_LENS_FLARE;
        else if(std::strcmp(tag_class, "device_light_fixture") == 0)
            return TAG_CLASS_DEVICE_LIGHT_FIXTURE;
        else if(std::strcmp(tag_class, "light") == 0)
            return TAG_CLASS_LIGHT;
        else if(std::strcmp(tag_class, "sound_looping") == 0)
            return TAG_CLASS_SOUND_LOOPING;
        else if(std::strcmp(tag_class, "device_machine") == 0)
            return TAG_CLASS_DEVICE_MACHINE;
        else if(std::strcmp(tag_class, "globals") == 0)
            return TAG_CLASS_GLOBALS;
        else if(std::strcmp(tag_class, "meter") == 0)
            return TAG_CLASS_METER;
        else if(std::strcmp(tag_class, "light_volume") == 0)
            return TAG_CLASS_LIGHT_VOLUME;
        else if(std::strcmp(tag_class, "gbxmodel") == 0)
            return TAG_CLASS_GBXMODEL;
        else if(std::strcmp(tag_class, "model") == 0)
            return TAG_CLASS_MODEL;
        else if(std::strcmp(tag_class, "multiplayer_scenario_description") == 0)
            return TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION;
        else if(std::strcmp(tag_class, "preferences_network_game") == 0)
            return TAG_CLASS_PREFERENCES_NETWORK_GAME;
        else if(std::strcmp(tag_class, "object") == 0)
            return TAG_CLASS_OBJECT;
        else if(std::strcmp(tag_class, "particle") == 0)
            return TAG_CLASS_PARTICLE;
        else if(std::strcmp(tag_class, "particle_system") == 0)
            return TAG_CLASS_PARTICLE_SYSTEM;
        else if(std::strcmp(tag_class, "physics") == 0)
            return TAG_CLASS_PHYSICS;
        else if(std::strcmp(tag_class, "placeholder") == 0)
            return TAG_CLASS_PLACEHOLDER;
        else if(std::strcmp(tag_class, "point_physics") == 0)
            return TAG_CLASS_POINT_PHYSICS;
        else if(std::strcmp(tag_class, "projectile") == 0)
            return TAG_CLASS_PROJECTILE;
        else if(std::strcmp(tag_class, "weather") == 0)
            return TAG_CLASS_WEATHER_PARTICLE_SYSTEM;
        else if(std::strcmp(tag_class, "scenario_structure_bsp") == 0)
            return TAG_CLASS_SCENARIO_STRUCTURE_BSP;
        else if(std::strcmp(tag_class, "scenery") == 0)
            return TAG_CLASS_SCENERY;
        else if(std::strcmp(tag_class, "shader_transparent_chicago_extended") == 0)
            return TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED;
        else if(std::strcmp(tag_class, "shader_transparent_chicago") == 0)
            return TAG_CLASS_SHADER_TRANSPARENT_CHICAGO;
        else if(std::strcmp(tag_class, "scenario") == 0)
            return TAG_CLASS_SCENARIO;
        else if(std::strcmp(tag_class, "shader_environment") == 0)
            return TAG_CLASS_SHADER_ENVIRONMENT;
        else if(std::strcmp(tag_class, "transparent_glass") == 0)
            return TAG_CLASS_SHADER_TRANSPARENT_GLASS;
        else if(std::strcmp(tag_class, "shader") == 0)
            return TAG_CLASS_SHADER;
        else if(std::strcmp(tag_class, "sky") == 0)
            return TAG_CLASS_SKY;
        else if(std::strcmp(tag_class, "shader_transparent_meter") == 0)
            return TAG_CLASS_SHADER_TRANSPARENT_METER;
        else if(std::strcmp(tag_class, "sound") == 0)
            return TAG_CLASS_SOUND;
        else if(std::strcmp(tag_class, "sound_environment") == 0)
            return TAG_CLASS_SOUND_ENVIRONMENT;
        else if(std::strcmp(tag_class, "shader_model") == 0)
            return TAG_CLASS_SHADER_MODEL;
        else if(std::strcmp(tag_class, "shader_transparent_generic") == 0)
            return TAG_CLASS_SHADER_TRANSPARENT_GENERIC;
        else if(std::strcmp(tag_class, "ui_widget_collection") == 0)
            return TAG_CLASS_UI_WIDGET_COLLECTION;
        else if(std::strcmp(tag_class, "shader_transparent_plasma") == 0)
            return TAG_CLASS_SHADER_TRANSPARENT_PLASMA;
        else if(std::strcmp(tag_class, "sound_scenery") == 0)
            return TAG_CLASS_SOUND_SCENERY;
        else if(std::strcmp(tag_class, "string_list") == 0)
            return TAG_CLASS_STRING_LIST;
        else if(std::strcmp(tag_class, "shader_transparent_water") == 0)
            return TAG_CLASS_SHADER_TRANSPARENT_WATER;
        else if(std::strcmp(tag_class, "tag_collection") == 0)
            return TAG_CLASS_TAG_COLLECTION;
        else if(std::strcmp(tag_class, "camera_track") == 0)
            return TAG_CLASS_CAMERA_TRACK;
        else if(std::strcmp(tag_class, "unit_dialogue") == 0)
            return TAG_CLASS_DIALOGUE;
        else if(std::strcmp(tag_class, "unit_hud_interface") == 0)
            return TAG_CLASS_UNIT_HUD_INTERFACE;
        else if(std::strcmp(tag_class, "unit") == 0)
            return TAG_CLASS_UNIT;
        else if(std::strcmp(tag_class, "unicode_string_list") == 0)
            return TAG_CLASS_UNICODE_STRING_LIST;
        else if(std::strcmp(tag_class, "virtual_keyboard") == 0)
            return TAG_CLASS_VIRTUAL_KEYBOARD;
        else if(std::strcmp(tag_class, "vehicle") == 0)
            return TAG_CLASS_VEHICLE;
        else if(std::strcmp(tag_class, "weapon") == 0)
            return TAG_CLASS_WEAPON;
        else if(std::strcmp(tag_class, "wind") == 0)
            return TAG_CLASS_WIND;
        else if(std::strcmp(tag_class, "weapon_hud_interface") == 0)
            return TAG_CLASS_WEAPON_HUD_INTERFACE;
        else
            return TAG_CLASS_NULL;
    }
}