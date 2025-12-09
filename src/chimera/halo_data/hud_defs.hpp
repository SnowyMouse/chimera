// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HUD_DEFS_HPP
#define CHIMERA_HUD_DEFS_HPP

#include <cstdint>
#include "type.hpp"
#include "tag.hpp"
#include "pad.hpp"
#include "shader_defs.hpp"

namespace Chimera {

    enum HUDFlashFLags {
        HUD_FLASH_FLAGS_REVERSE_COLORS_BIT,
        NUMBER_OF_HUD_FLASH_FLAGS
    };

    enum HUDScaleFlags {
        HUD_SCALE_FLAGS_DONT_SCALE_OFFSET_BIT,
        HUD_SCALE_FLAGS_DONT_SCALE_SIZE_BIT,
        HUD_SCALE_FLAGS_USE_HIGHRES_SCALE_BIT,
        NUMBER_OF_HUD_SCALE_FLAGS
    };

    enum HUDWaypointFlags {
        HUD_WAYPOINT_FLAGS_DONT_ROTATE_OFFSCREEN_BIT,
        NUMBER_OF_WAYPOINT_FLAGS
    };

    enum HUDIconFlags {
        HUD_ICON_FLAGS_USE_TEXT_BIT,
        HUD_ICON_FLAGS_USE_COLOR_BIT,
        HUD_ICON_FLAGS_ABSOLUTE_WIDTH_BIT,
        NUMBER_OF_ICON_FLAGS
    };

    enum HUDMeterFlags{
        HUD_METER_FLAGS_USE_MIN_MAX_FOR_STATE_CHANGES_BIT,
        HUD_METER_FLAGS_INTERPOLATE_BETWEEN_MIN_MAX_BIT,
        HUD_METER_FLAGS_INTERPOLATE_IN_HSV_SPACE_BIT,
        HUD_METER_FLAGS_INTERPOLATE_ALONG_FARTHEST_HUE_PATH_BIT,
        HUD_METER_FLAGS_INVERT_INTERPOLATION_VALUE_BIT,
        HUD_METER_FLAGS_USE_XBOX_SHADING_BIT,
        NUMBER_OF_HUD_METER_FLAGS,
    };

    enum HUDNumberFlags {
        HUD_NUMBER_FLAGS_SHOW_ALL_LEADING_ZEROS_BIT,
        HUD_NUMBER_FLAGS_SHOW_ONLY_WHEN_ZOOMED,
        HUD_NUMBER_FLAGS_SHOW_TRAILING_M,
        NUMBER_OF_HUD_NUMBER_FLAGS
    };

    enum HUDScreenEffectMaskFlags {
        HUD_SCREEN_EFFECT_MASK_FLAGS_ONLY_WHEN_ZOOMED_BIT,
        NUMBER_OF_HUD_SCREEN_EFFECT_MASK_FLAGS
    };

    enum HUDScreenEffectConvolutionFlags {
        HUD_SCREEN_EFFECT_CONVOLUTION_FLAGS_ONLY_WHEN_ZOOMED_BIt,
        NUMBER_OF_HUD_SCREEN_EFFECT_CONVOLUTION_FLAGS
    };

    enum HUDScreenEffectLightEnhancementFlags {
        HUD_SCREEN_EFFECT_LIGHT_ENHANCEMENT_FLAGS_ONLY_WHEN_ZOOMED_BIT,
        HUD_SCREEN_EFFECT_LIGHT_ENHANCEMENT_FLAGS_CONNECT_TO_FLASHLIGHT_BIT,
        HUD_SCREEN_EFFECT_LIGHT_ENHANCEMENT_FLAGS_USES_CONVOLUTION_MASK_BIT,
        NUMBER_OF_HUD_SCREEN_EFFECT_LIGHT_ENHANCEMENT_FLAGS
    };

    enum HUDScreenEffectDesaturationFlags {
        HUD_SCREEN_EFFECT_DESATURATION_FLAGS_ONLY_WHEN_ZOOMED_BIT,
        HUD_SCREEN_EFFECT_DESATURATION_FLAGS_CONNECT_TO_FLASHLIGHT_BIT,
        HUD_SCREEN_EFFECT_DESATURATION_FLAGS_IS_ADDITIVE_BIT,
        HUD_SCREEN_EFFECT_DESATURATION_FLAGS_USES_CONVOLUTION_MASK_BIT,
        NUMBER_OF_HUD_SCREEN_EFFECT_DESATURATION_FLAGS
    };

    enum GrenadeHUDInterfaceOverlayFlags {
        GRENADE_HUD_INTERFACE_OVERLAY_FLAGS_ON_FLASHING_BIT,
        GRENADE_HUD_INTERFACE_OVERLAY_FLAGS_ON_EMPTY_BIT,
        GRENADE_HUD_INTERFACE_OVERLAY_FLAGS_ON_DEFAULT_BIT,
        GRENADE_HUD_INTERFACE_OVERLAY_FLAGS_ON_ALWAYS_BIT,
        NUMBER_OF_GRENADE_HUD_INTERFACE_OVERLAY_FLAGS
    };

    enum WeaponHUDInterfaceOverlayTypeFlags {
        WEAPON_HUD_INTERFACE_OVERLAY_TYPE_FLAGS_ON_FLASHING_BIT,
        WEAPON_HUD_INTERFACE_OVERLAY_TYPE_FLAGS_ON_EMPTY_BIT,
        WEAPON_HUD_INTERFACE_OVERLAY_TYPE_FLAGS_ON_RELOAD_BIT,
        WEAPON_HUD_INTERFACE_OVERLAY_TYPE_FLAGS_ON_DEFAULT_BIT,
        WEAPON_HUD_INTERFACE_OVERLAY_TYPE_FLAGS_ON_ALWAYS_BIT,
        NUMBER_OF_WEAPON_HUD_INTERFACE_OVERLAY_TYPE_FLAGS
    };

    enum WeaponHUDInterfaceOverlayFlags {
        WEAPON_HUD_INTERFACE_OVERLAY_FLAGS_FLASHES_BIT,
        WEAPON_HUD_INTERFACE_OVERLAY_FLAGS_RUNTIME_INVALID_BIT,
        NUMBER_OF_WEAPON_HUD_INTERFACE_OVERLAY_FLAGS
    };

    enum WeaponHUDInterfaceCrosshairFlags {
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_FLASHES_BIT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_NOT_A_SPRITE_BIT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_NOT_ON_DEFAULT_ZOOM_BIT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_SHOW_SNIPER_DATA_BIT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_HIDE_OUTSIDE_AREA_BIT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_ONE_ZOOM_LEVEL_BIT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_ONLY_ON_DEFAULT_ZOOM_BIT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS_RUNTIME_INVALID_BIT,
        NUMBER_OF_WEAPON_HUD_INTERFACE_CROSSHAIR_FLAGS
    };

    enum HUDRuntimeFlags {
        HUD_RUNTIME_FLAGS_INVALID_BIT,
        NUMBER_OF_HUD_RUNTIME_FLAGS
    };

    enum WeaponHUDInterfaceFlashingFlags {
        WEAPON_HUD_INTERFACE_FLASHING_FLAGS_USE_PARENT_FLASH_PARAMETERS_BIT,
        NUMBER_OF_WEAPON_HUD_INTERFACE_FLASHING_FLAGS
    };

    enum WeaponHUDInterfaceNumberFlags {
        WEAPON_HUD_INTERFACE_NUMBER_FLAGS_DIVIDE_BY_CLIP_SIZE_BIT,
        NUMBER_OF_WEAPON_HUD_INTERFACE_NUMBER_FLAGS
    };

    enum HUDAnchor : std::uint16_t {
        HUD_ANCHOR_TOP_LEFT,
        HUD_ANCHOR_TOP_RIGHT,
        HUD_ANCHOR_BOTTOM_LEFT,
        HUD_ANCHOR_BOTTOM_RIGHT,
        HUD_ANCHOR_CENTER,
        HUD_ANCHOR_TOP_CENTER,
        HUD_ANCHOR_BOTTOM_CENTER,
        HUD_ANCHOR_LEFT_CENTER,
        HUD_ANCHOR_RIGHT_CENTER,
        NUMBER_OF_HUD_ANCHORS
    };

    enum HUDChildAnchor : std::uint16_t {
        HUD_CHILD_ANCHOR_FROM_PARENT,
        HUD_CHILD_ANCHOR_TOP_LEFT,
        HUD_CHILD_ANCHOR_TOP_RIGHT,
        HUD_CHILD_ANCHOR_BOTTOM_LEFT,
        HUD_CHILD_ANCHOR_BOTTOM_RIGHT,
        HUD_CHILD_ANCHOR_CENTER,
        HUD_CHILD_ANCHOR_TOP_CENTER,
        HUD_CHILD_ANCHOR_BOTTOM_CENTER,
        HUD_CHILD_ANCHOR_LEFT_CENTER,
        HUD_CHILD_ANCHOR_RIGHT_CENTER,
        NUMBER_OF_HUD_CHILD_ANCHORS
    };

    enum HUDCanvasSize : std::uint16_t {
        HUD_CANVAS_SIZE_480P,
        HUD_CANVAS_SIZE_960P,
        NUMBER_OF_HUD_CANVAS_SIZES
    };

    enum HUDUseOnMapType : std::uint16_t {
        HUD_USE_ON_MAP_TYPE_ANY,
        HUD_USE_ON_MAP_TYPE_SOLO,
        HUD_USE_ON_MAP_TYPE_MULTIPLAYER,
        NUMBER_OF_HUD_USE_ON_MAP_TYPES
    };

    enum HUDBlipType : std::uint16_t {
        HUD_BLIP_TYPE_MEDIUM,
        HUD_BLIP_TYPE_SMALL,
        HUD_BLIP_TYPE_LARGE,
        NUMBER_OF_HUD_BLIP_TYPES
    };

    enum HUDMultitextureOverlayEffectorType : std::uint16_t {
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_TYPE_TINT,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_TYPE_HORIZONTAL_OFFSET,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_TYPE_VERTICAL_OFFSET,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_TYPE_ALPHA,
        NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_EFFECTOR_TYPES
    };

    enum HUDMultitextureOverlayEffectorDestination : std::uint16_t {
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_DESTINATION_GEOMETRY_OFFSET,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_DESTINATION_PRIMARY_MAP,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_DESTINATION_SECONDARY_MAP,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_DESTINATION_TERTIARY_MAP,
        NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_EFFECTOR_DESTINATIONS
    };

    enum HUDMultitextureOverlayEffectorSource : std::uint16_t {
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_PLAYER_PITCH,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_PLAYER_PITCH_TANGENT,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_PLAYER_YAW,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_WEAPON_AMMO_LOADED,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_WEAPON_AMMO_TOTAL,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_WEAPON_HEAT,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_EXPLICIT,
        HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCE_ZOOM_LEVEL,
        NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCES
    };

    enum HUDMultitextureOverlayBlendFunction : std::uint16_t {
        HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_ADD,
        HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_SUBTRACT,
        HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_MULTIPLY,
        HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_MULTIPLY2X,
        HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_DOT,
        NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTIONS
    };

    enum HUDMultitextureOverlayAnchor : std::uint16_t {
        HUD_MULTITEXTURE_OVERLAY_ANCHOR_TEXTURE,
        HUD_MULTITEXTURE_OVERLAY_ANCHOR_SCREEN,
        NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_ANCHORS
    };

    enum HUDMultitextureOverlayAddressMode : std::uint16_t {
        HUD_MULTITEXTURE_OVERLAY_ADDRESS_MODE_CLAMP,
        HUD_MULTITEXTURE_OVERLAY_ADDRESS_MODE_WRAP,
        NUMBER_OF_MULTITEXTURE_ADDRESS_MODES
    };

    enum HUDWaypointType : std::uint16_t {
        HUD_WAYPOINT_TYPE_ON_SCREEN,
        HUD_WAYPOINT_TYPE_OFF_SCREEN,
        HUD_WAYPOINT_TYPE_OCCLUDED,
        NUMBER_OF_HUD_WAYPOINT_TYPES
    };

    enum WeaponHUDInterfaceState : std::uint16_t {
        WEAPON_HUD_INTERFACE_STATE_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_STATE_LOADED_AMMO,
        WEAPON_HUD_INTERFACE_STATE_HEAT,
        WEAPON_HUD_INTERFACE_STATE_AGE,
        WEAPON_HUD_INTERFACE_STATE_SECONDARY_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_STATE_SECONDARY_LOADED_AMMO,
        WEAPON_HUD_INTERFACE_STATE_DISTANCE_TO_TARGET,
        WEAPON_HUD_INTERFACE_STATE_ELEVATION_TO_TARGET,
        NUMBER_OF_WEAPON_HUD_INTERFACE_STATES
    };

    enum WeaponHUDInterfaceCrosshairType : std::uint16_t {
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_AIM,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_ZOOM,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_CHARGE,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_HEAT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_TOTAL_BATTERY,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FIRED_WITH_NO_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_THREW_WITH_NO_GRENADE,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_AMMO_NONE_FOR_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_SECONDARY_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_SECONDARY_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_SECONDARY_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FIRED_SECONDARY_WITH_NO_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_SECONDARY_AMMO_NONE_FOR_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_PRIMARY_TRIGGER_READY,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_SECONDARY_TRIGGER_READY,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_FIRED_BATTERY_DEPLETED,
        NUMBER_OF_WEAPON_HUD_INTERFACE_CROSSHAIR_TYPES
    };

    struct HUDAbsolutePlacement {
        HUDAnchor anchor;
        HUDCanvasSize canvas_size;
        PAD(0x20);
    };
    static_assert(sizeof(HUDAbsolutePlacement) == 0x24);

    struct HUDPlacement {
        Vector2DInt offset;
        VectorIJ scale;
        std::uint16_t scaling_flags;
        PAD(0x16);
    };
    static_assert(sizeof(HUDPlacement) == 0x24);

    struct HUDObjective {
        std::int16_t up_ticks;
        std::int16_t fade_ticks;
    };
    static_assert(sizeof(HUDObjective) == 0x4);

    struct HUDColor {
        Pixel32 color;
        Pixel32 flash_color;
        float flash_period;
        float flash_delay;
        std::int16_t number_of_flashes;
        std::uint16_t flash_flags;
        float flash_length;
        Pixel32 disabled_color;
        HUDObjective objective;
    };
    static_assert(sizeof(HUDColor) == 0x20);

    struct HUDStaticElement {
        HUDPlacement placement;
        TagReference interface_bitmap;
        HUDColor colors;
        std::int16_t sequence_index;
        PAD(0x2);
        TagBlock multitexture_overlays;
        PAD(0x4);
    };
    static_assert(sizeof(HUDStaticElement) == 0x68);

    struct HUDMeterElement {
        HUDPlacement placement;
        TagReference meter_bitmap;
        Pixel32 min_color;
        Pixel32 max_color;
        Pixel32 flash_color;
        Pixel32 empty_color;
        std::uint8_t meter_flags;
        std::uint8_t minimum_value;
        std::int16_t sequence_index;
        std::uint8_t alpha_multiplier;
        std::uint8_t alpha_bias;
        std::int16_t value_scale;
        float opacity;
        float fade;
        Pixel32 disabled_color;
        float min_alpha;
        PAD(0xC);
    };
    static_assert(sizeof(HUDMeterElement) == 0x68);

    struct HUDNumberElement {
        HUDPlacement placement;
        HUDColor colors;
        std::int8_t digits;
        std::uint8_t number_flags;
        std::int8_t fractional_digits;
        PAD(0xD);
    };
    static_assert(sizeof(HUDNumberElement) == 0x54);

    struct HUDIconElement {
        std::int16_t sequence_index;
        std::int16_t width_offset;
        Vector2DInt offset;
        Pixel32 color;
        std::int8_t frame_rate;
        std::uint8_t flags;
        std::int16_t text_index;
    };
    static_assert(sizeof(HUDIconElement) == 0x10);

    struct HUDScreenEffect {
        PAD(0x4);
        std::uint16_t mask_flags;
        PAD(0x12);
        TagReference mask_fullscreen;
        TagReference mask_splitscreen;
        PAD(0x8);
        std::uint16_t convolution_flags;
        PAD(0x2);
        float convolution_radius_in_bounds[2];
        float convolution_radius_out_bounds[2];
        PAD(0x18);
        std::uint16_t light_enhancement_flags;
        std::int16_t light_enhancement_script_source;
        float light_enhancement_intensity;
        PAD(0x18);
        std::uint16_t desaturation_flags;
        std::int16_t desaturation_script_source;
        float desaturation_intensity;
        ColorRGB desaturation_tint;
        PAD(0x18);
    };
    static_assert(sizeof(HUDScreenEffect) == 0xB8);

    struct HUDMultitextureOverlayEffector {
        PAD(0x40);
        HUDMultitextureOverlayEffectorType destination_type;
        HUDMultitextureOverlayEffectorDestination destination;
        HUDMultitextureOverlayEffectorSource source;
        PAD(0x2);
        float in_bounds[2];
        float out_bounds[2];
        PAD(0x40);
        ColorRGB tint_color_lower_bounds;
        ColorRGB tint_color_upper_bounds;
        std::int16_t periodic_function;
        PAD(0x2);
        float periodic_function_period;
        float periodic_function_phase;
        PAD(0x20);
    };
    static_assert(sizeof(HUDMultitextureOverlayEffector) == 0xDC);

    struct HUDMultitextureOverlay {
        std::uint16_t flags;
        std::int16_t type;
        FramebufferBlendFunction framebuffer_blend_function;
        PAD(0x22);
        HUDMultitextureOverlayAnchor map_anchor[3];
        HUDMultitextureOverlayBlendFunction map_blending_function[2];
        PAD(0x2);
        VectorIJ map_scale[3];
        VectorIJ map_offset[3];
        TagReference map[3];
        HUDMultitextureOverlayAddressMode map_clamp[3];
        PAD(0xBA);
        TagBlock effectors;
        PAD(0x80);
    };
    static_assert(sizeof(HUDMultitextureOverlay) == 0x1E0);

    struct HUDMessagingParameters {
        HUDAbsolutePlacement absolute_placement;
        HUDPlacement placement;
        TagReference fullscreen_font;
        TagReference splitscreen_font;
        float up_time;
        float fade_time;
        ColorARGB state_color;
        ColorARGB text_color;
        float spacing;
        TagReference hud_item_messages;
        TagReference messaging_icons;
        TagReference alternate_icon_text;
        TagBlock button_icons;
        HUDColor color;
        TagReference hud_messages;
        HUDColor objective_color;
    };
    static_assert(sizeof(HUDMessagingParameters) == 0x120);

    struct HUDWaypoint {
        float top_offset;
        float bottom_offset;
        float left_offset;
        float right_offset;
        PAD(0x20);
        TagReference arrow_bitmap;
        TagBlock arrows;
        PAD(0x50);
    };
    static_assert(sizeof(HUDWaypoint) == 0x9C);

    struct HUDWaypointArrow {
        TagString name;
        PAD(0x8);
        Pixel32 color;
        float opacity;
        float fade;
        std::int16_t sequence_indices[NUMBER_OF_HUD_WAYPOINT_TYPES];
        PAD(0x12);
        std::uint32_t flags;
        PAD(0x18);
    };
    static_assert(sizeof(HUDWaypointArrow) == 0x68);

    struct HUDMultiplayerParameters {
        float hud_scale;
        PAD(0x100);
    };
    static_assert(sizeof(HUDMultiplayerParameters) == 0x104);

    struct HUDDefaults {
        TagReference default_weapon_hud;
        float motion_sensor_range;
        float motion_sensor_velocity_sensitivity;
        float motion_sensor_scale;
        Rectangle2D default_title_bounds;
        PAD(0x2C);
    };
    static_assert(sizeof(HUDDefaults) == 0x50);

    struct HUDDamageIndicators {
        std::int16_t top_offset;
        std::int16_t bottom_offset;
        std::int16_t left_offset;
        std::int16_t right_offset;
        PAD(32);
        TagReference indicator_bitmap;
        std::int16_t sequence_index;
        std::int16_t multiplayer_sequence_index;
        Pixel32 color;
        PAD(16);
    };
    static_assert(sizeof(HUDDamageIndicators) == 0x50);

    struct HUDTimer {
        HUDColor color;
        HUDColor time_up_color;
        PAD(0x28);
    };
    static_assert(sizeof(HUDTimer) == 0x68);

    struct HUDGlobals {
        HUDMessagingParameters messaging;
        HUDWaypoint waypoint;
        HUDMultiplayerParameters multiplayer;
        HUDDefaults defaults;
        HUDDamageIndicators damage_indicators;
        HUDTimer timer_definition;
        TagReference carnage_report_bitmap;
        std::int16_t loading_begin_index;
        std::int16_t loading_end_index;
        std::int16_t checkpoint_begin_index;
        std::int16_t checkpoint_end_index;
        TagReference checkpoint_sound;
        PAD(0x60);
    };
    static_assert(sizeof(HUDGlobals) == 0x450);

    struct HUDNumber {
        TagReference number_bitmap;
        std::int8_t character_width;
        std::int8_t screen_width;
        std::int8_t x_offset;
        std::int8_t y_offset;
        std::int8_t decimal_point_width;
        std::int8_t colon_width;
        PAD(0x4E);
    };
    static_assert(sizeof(HUDNumber) == 0x64);

    struct WeaponHUDInterfaceElementHeader {
        WeaponHUDInterfaceState state_type;
        std::uint16_t runtime_flags;
        HUDUseOnMapType use_on_map_type;
        HUDChildAnchor child_anchor;
        PAD(0x1C);
    };
    static_assert(sizeof(WeaponHUDInterfaceElementHeader) == 0x24);

    struct WeaponHUDInterfaceOverlay {
        TagReference bitmap;
        TagBlock items;
    };
    static_assert(sizeof(WeaponHUDInterfaceOverlay) == 0x1C);

    struct WeaponHUDInterfaceOverlayItem {
        HUDPlacement placement;
        HUDColor colors;
        std::int16_t frame_rate;
        PAD(0x2);
        std::int16_t sequence_index;
        std::uint16_t type;
        std::uint32_t flags;
        PAD(0x38);
    };
    static_assert(sizeof(WeaponHUDInterfaceOverlayItem) == 0x88);

    struct WeaponHUDInterfaceFlashState {
        std::uint16_t flags;
        PAD(0x2);
        std::int16_t total_ammo;
        std::int16_t loaded_ammo;
        std::int16_t heat;
        std::int16_t age;
        PAD(0x20);
    };
    static_assert(sizeof(WeaponHUDInterfaceFlashState) == 0x2C);

    struct WeaponHUDInterfaceStaticElement {
        WeaponHUDInterfaceElementHeader header;
        HUDStaticElement static_element;
        PAD(0x28);
    };
    static_assert(sizeof(WeaponHUDInterfaceStaticElement) == 0xB4);


    struct WeaponHUDInterfaceMeterElement {
        WeaponHUDInterfaceElementHeader header;
        HUDMeterElement meter_element;
        PAD(0x28);
    };
    static_assert(sizeof(WeaponHUDInterfaceMeterElement) == 0xB4);

    struct WeaponHUDInterfaceNumberElement {
        WeaponHUDInterfaceElementHeader header;
        HUDNumberElement number_element;
        std::uint16_t weapon_flags;
        PAD(0x26);
    };
    static_assert(sizeof(WeaponHUDInterfaceNumberElement) == 0xA0);

    struct WeaponHUDInterfaceOverlaysElement {
        WeaponHUDInterfaceElementHeader header;
        WeaponHUDInterfaceOverlay overlays;
        PAD(0x28);
    };
    static_assert(sizeof(WeaponHUDInterfaceOverlaysElement) == 0x68);

    struct WeaponHUDInterfaceCrosshairItem {
        HUDPlacement placement;
        HUDColor colors;
        std::int16_t frame_rate;
        std::int16_t sequence_index;
        std::uint32_t flags;
        PAD(0x20);
    };
    static_assert(sizeof(WeaponHUDInterfaceCrosshairItem) == 0x6C);

    struct WeaponHUDInterfaceCrosshair {
        TagReference bitmap;
        TagBlock items;
    };
    static_assert(sizeof(WeaponHUDInterfaceCrosshair) == 0x1C);

    struct WeaponHUDInterfaceCrosshairElement {
        WeaponHUDInterfaceCrosshairType crosshair_type;
        std::uint16_t runtime_flags;
        HUDUseOnMapType use_on_map_type;
        PAD(0x1E);
        WeaponHUDInterfaceCrosshair crosshairs;
        PAD(0x28);
    };
    static_assert(sizeof(WeaponHUDInterfaceCrosshairElement) == 0x68);

    struct WeaponHUDInterface {
        TagReference parent_hud;
        WeaponHUDInterfaceFlashState flash_cutoffs;
        HUDAbsolutePlacement absolute_placement;
        TagBlock statics;
        TagBlock meters;
        TagBlock numbers;
        TagBlock crosshairs;
        TagBlock overlays;
        std::uint32_t valid_crosshair_types_flags;
        PAD(0xC);
        TagBlock screen_effects;
        PAD(0x84);
        HUDIconElement messaging_icon;
        PAD(0x30);
    };
    static_assert(sizeof(WeaponHUDInterface) == 0x17C);

}

#endif
