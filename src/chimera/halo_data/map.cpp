// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "map.hpp"
#include "tag.hpp"
#include "tag_class.hpp"
#include "game_engine.hpp"
#include <optional>

namespace Chimera {
    MapHeader &get_map_header() noexcept {
        static auto *map_header = *reinterpret_cast<MapHeader **>(get_chimera().get_signature("map_header_sig").data() + 2);
        return *map_header;
    }

    MapHeaderDemo &get_demo_map_header() noexcept {
        auto &map_header_sig = get_chimera().get_signature("map_header_sig");
        static auto *map_header = reinterpret_cast<MapHeaderDemo *>(*reinterpret_cast<std::byte **>(map_header_sig.data() + 2) - 0x2C0);
        return *map_header;
    }

    MapList &get_map_list() noexcept {
        static std::optional<MapList *> all_map_indices;
        if(!all_map_indices.has_value()) {
            if(game_engine() == GAME_ENGINE_DEMO) {
                all_map_indices = *reinterpret_cast<MapList **>(get_chimera().get_signature("map_index_demo_sig").data() + 2);
            }
            else {
                all_map_indices = *reinterpret_cast<MapList **>(get_chimera().get_signature("map_index_sig").data() + 10);
            }
        }
        return **all_map_indices;
    }
    
    template <typename Header> static bool check_valid_header(const Header &header) {
        return header.head == Header::HEAD_LITERAL && header.foot == Header::FOOT_LITERAL;
    }
    
    bool MapHeader::is_valid() const noexcept {
        return check_valid_header(*this);
    }
    
    bool MapHeaderDemo::is_valid() const noexcept {
        return check_valid_header(*this);
    }

    // hack from Invader
    bool map_is_protected() noexcept {
        auto &tag_data_header = get_tag_data_header();

        // Get the scenario tag (not always the first tag) and make sure it's actually a scenario tag
        auto *scenario_tag = get_tag(tag_data_header.scenario_tag.index.index);
        if(scenario_tag->primary_class != TagClassInt::TAG_CLASS_SCENARIO) {
            return true;
        }

        // Get the tag count and friends
        auto tag_count = tag_data_header.tag_count;
        auto *base_address = reinterpret_cast<const char *>(&tag_data_header);
        auto *base_address_end = base_address + 0x1700000; // a little shorter, but we know that this region is safe and that tag paths almost certainly cannot appear here

        for(std::size_t t = 0; t < tag_count; t++) {
            auto *tag = get_tag(t);

            auto *tag_path = tag->path;
            auto tag_class = tag->primary_class;

            // Bounds check the tag path so we don't blow up
            if(tag_path < base_address || tag_path >= base_address_end) {
                return true;
            }

            // Bounds check the end so strcmp doesn't explode
            for(auto *tp = tag_path; *tp; tp++) {
                if(tp + 1 >= base_address_end) {
                    return true;
                }
            }

            // And of course check if it's empty
            if(std::strcmp(tag_path, "") == 0) {
                return true;
            }

            // If it says it's a BSP, check if it's really a BSP
            if(tag->primary_class == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                auto *scenario_struct = scenario_tag->data;
                auto scenario_bsp_count = *reinterpret_cast<const std::uint32_t *>(scenario_struct + 0x5A4);
                auto *scenario_bsp_data = *reinterpret_cast<const std::byte * const *>(scenario_struct + 0x5A4 + 4);
                bool actually_a_bsp = false;
                for(std::uint32_t i = 0; i < scenario_bsp_count && !actually_a_bsp; i++) {
                    actually_a_bsp = *reinterpret_cast<const std::uint16_t *>(scenario_bsp_data + i * 0x20 + 0x10 + 0xC) == t;
                }
                if(!actually_a_bsp) {
                    return true;
                }
            }

            // Account for Missingno.'s from Invader
            if(reinterpret_cast<std::uint32_t>(tag->data) == 0xFFFFFFFF && tag->primary_class != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                continue;
            }

            // Check if the tag class is valid
            switch(tag_class) {
                case TagClassInt::TAG_CLASS_ACTOR:
                case TagClassInt::TAG_CLASS_ACTOR_VARIANT:
                case TagClassInt::TAG_CLASS_ANTENNA:
                case TagClassInt::TAG_CLASS_MODEL_ANIMATIONS:
                case TagClassInt::TAG_CLASS_BIPED:
                case TagClassInt::TAG_CLASS_BITMAP:
                case TagClassInt::TAG_CLASS_SPHEROID:
                case TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
                case TagClassInt::TAG_CLASS_MODEL_COLLISION_GEOMETRY:
                case TagClassInt::TAG_CLASS_COLOR_TABLE:
                case TagClassInt::TAG_CLASS_CONTRAIL:
                case TagClassInt::TAG_CLASS_DEVICE_CONTROL:
                case TagClassInt::TAG_CLASS_DECAL:
                case TagClassInt::TAG_CLASS_UI_WIDGET_DEFINITION:
                case TagClassInt::TAG_CLASS_INPUT_DEVICE_DEFAULTS:
                case TagClassInt::TAG_CLASS_DEVICE:
                case TagClassInt::TAG_CLASS_DETAIL_OBJECT_COLLECTION:
                case TagClassInt::TAG_CLASS_EFFECT:
                case TagClassInt::TAG_CLASS_EQUIPMENT:
                case TagClassInt::TAG_CLASS_FLAG:
                case TagClassInt::TAG_CLASS_FOG:
                case TagClassInt::TAG_CLASS_FONT:
                case TagClassInt::TAG_CLASS_MATERIAL_EFFECTS:
                case TagClassInt::TAG_CLASS_GARBAGE:
                case TagClassInt::TAG_CLASS_GLOW:
                case TagClassInt::TAG_CLASS_GRENADE_HUD_INTERFACE:
                case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT:
                case TagClassInt::TAG_CLASS_HUD_NUMBER:
                case TagClassInt::TAG_CLASS_HUD_GLOBALS:
                //case TagClassInt::TAG_CLASS_ITEM:
                case TagClassInt::TAG_CLASS_ITEM_COLLECTION:
                case TagClassInt::TAG_CLASS_DAMAGE_EFFECT:
                case TagClassInt::TAG_CLASS_LENS_FLARE:
                case TagClassInt::TAG_CLASS_LIGHTNING:
                case TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE:
                case TagClassInt::TAG_CLASS_LIGHT:
                case TagClassInt::TAG_CLASS_SOUND_LOOPING:
                case TagClassInt::TAG_CLASS_DEVICE_MACHINE:
                case TagClassInt::TAG_CLASS_GLOBALS:
                case TagClassInt::TAG_CLASS_METER:
                case TagClassInt::TAG_CLASS_LIGHT_VOLUME:
                case TagClassInt::TAG_CLASS_GBXMODEL:
                //case TagClassInt::TAG_CLASS_MODEL:
                case TagClassInt::TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION:
                case TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
                //case TagClassInt::TAG_CLASS_OBJECT:
                case TagClassInt::TAG_CLASS_PARTICLE:
                case TagClassInt::TAG_CLASS_PARTICLE_SYSTEM:
                case TagClassInt::TAG_CLASS_PHYSICS:
                case TagClassInt::TAG_CLASS_PLACEHOLDER:
                case TagClassInt::TAG_CLASS_POINT_PHYSICS:
                case TagClassInt::TAG_CLASS_PROJECTILE:
                case TagClassInt::TAG_CLASS_WEATHER_PARTICLE_SYSTEM:
                case TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP:
                case TagClassInt::TAG_CLASS_SCENERY:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO:
                case TagClassInt::TAG_CLASS_SCENARIO:
                case TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS:
                //case TagClassInt::TAG_CLASS_SHADER:
                case TagClassInt::TAG_CLASS_SKY:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER:
                case TagClassInt::TAG_CLASS_SOUND:
                case TagClassInt::TAG_CLASS_SOUND_ENVIRONMENT:
                case TagClassInt::TAG_CLASS_SHADER_MODEL:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC:
                case TagClassInt::TAG_CLASS_UI_WIDGET_COLLECTION:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA:
                case TagClassInt::TAG_CLASS_SOUND_SCENERY:
                case TagClassInt::TAG_CLASS_STRING_LIST:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_WATER:
                case TagClassInt::TAG_CLASS_TAG_COLLECTION:
                case TagClassInt::TAG_CLASS_CAMERA_TRACK:
                case TagClassInt::TAG_CLASS_DIALOGUE:
                case TagClassInt::TAG_CLASS_UNIT_HUD_INTERFACE:
                //case TagClassInt::TAG_CLASS_UNIT:
                case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST:
                case TagClassInt::TAG_CLASS_VIRTUAL_KEYBOARD:
                case TagClassInt::TAG_CLASS_VEHICLE:
                case TagClassInt::TAG_CLASS_WEAPON:
                case TagClassInt::TAG_CLASS_WIND:
                case TagClassInt::TAG_CLASS_WEAPON_HUD_INTERFACE:
                    break;
                default:
                    return true;
            }

            // Check all past paths to see if we have a duplicate
            for(std::size_t t2 = 0; t2 < t; t2++) {
                auto *tag2 = get_tag(t2);
                if(tag_class != tag2->primary_class) {
                    continue;
                }

                // Account for Invader's missingno.
                if(reinterpret_cast<std::uint32_t>(tag2->data) == 0xFFFFFFFF && tag2->primary_class != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                    continue;
                }

                if(std::strcmp(tag_path, tag2->path) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    extern "C" void load_ui_map_asm() noexcept;
    void load_ui_map() noexcept {
        load_ui_map_asm();
    }
    
    const char *get_map_name() noexcept {
        return game_engine() == GameEngine::GAME_ENGINE_DEMO ? get_demo_map_header().name : get_map_header().name;
    }
}
