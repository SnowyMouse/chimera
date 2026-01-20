# SPDX-License-Identifier: GPL-3.0-only

include(src/chimera/command/command.cmake)

# Include version script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/version.hpp"
    COMMAND "${CMAKE_COMMAND}" "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}" "-DGIT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/.git" "-DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version.hpp" -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DIN_GIT_REPO=${IN_GIT_REPO} -P "${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/version.cmake"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/version.cmake" "$<$<BOOL:${IN_GIT_REPO}>:${CMAKE_CURRENT_SOURCE_DIR}/.git/refs/heads>"
)

# Make chimera-version
add_custom_target(chimera-version
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/version.hpp"
)

if(${CHIMERA_WINXP})
    set(WINXP_COMPATIBILITY_FILES
        src/chimera/map_loading/get_file_name_from_handle.c
    )

    set(WINXP_COMPATIBILITY_LIBRARIES
        psapi
    )

    set(D3DCOMPILER_LIBRARY
        d3dcompiler_43
    )
else()
    set(WINXP_COMPATIBILITY_FILES

    )
    set(WINXP_COMPATIBILITY_LIBRARIES

    )

    set(D3DCOMPILER_LIBRARY
        d3dcompiler
    )
endif()

# Define the source files
add_library(chimera STATIC
    ${WINXP_COMPATIBILITY_FILES}
    src/chimera/annoyance/auto_get_list.cpp
    src/chimera/annoyance/auto_get_list.S
    src/chimera/annoyance/drm.cpp
    src/chimera/annoyance/drm.S
    src/chimera/annoyance/exception_dialog.cpp
    src/chimera/annoyance/keystone.cpp
    src/chimera/annoyance/multi_instance.cpp
    src/chimera/annoyance/novideo.cpp
    src/chimera/annoyance/registry.cpp
    src/chimera/annoyance/tab_out_video.cpp
    src/chimera/annoyance/tab_out_video.S
    src/chimera/annoyance/update.cpp
    src/chimera/annoyance/update.S
    src/chimera/annoyance/watson.cpp
    src/chimera/bookmark/bookmark.cpp
    src/chimera/chimera.cpp
    src/chimera/command/command.cpp
    src/chimera/command/hotkey.cpp
    src/chimera/config/config.cpp
    src/chimera/config/ini.cpp
    src/chimera/console/console.cpp
    src/chimera/console/console.S
    src/chimera/custom_chat/custom_chat.cpp
    src/chimera/custom_chat/custom_chat.S
    src/chimera/event/camera.cpp
    src/chimera/event/command.cpp
    src/chimera/event/connect.cpp
    src/chimera/event/connect.S
    src/chimera/event/d3d9_end_scene.cpp
    src/chimera/event/d3d9_end_scene.S
    src/chimera/event/d3d9_reset.cpp
    src/chimera/event/d3d9_reset.S
    src/chimera/event/damage.cpp
    src/chimera/event/damage.S
    src/chimera/event/frame.cpp
    src/chimera/event/game_loop.cpp
    src/chimera/event/map_load.cpp
    src/chimera/event/revert.cpp
    src/chimera/event/rcon_message.cpp
    src/chimera/event/tick.cpp
    src/chimera/map_loading/crc32.c
    src/chimera/map_loading/fast_load.cpp
    src/chimera/map_loading/fast_load.S
    src/chimera/map_loading/laa.cpp
    src/chimera/fix/abolish_safe_mode.cpp
    src/chimera/fix/alternate_bump_attenuation.cpp
    src/chimera/fix/alternate_bump_attenuation.S
    src/chimera/fix/aim_assist.cpp
    src/chimera/fix/aim_assist.S
    src/chimera/fix/af.cpp
    src/chimera/fix/af.S
    src/chimera/fix/auto_center.cpp
    src/chimera/fix/bitmap_formats.cpp
    src/chimera/fix/blue_32bit_color_fix.cpp
    src/chimera/fix/bullshit_server_data.cpp
    src/chimera/fix/bullshit_server_data.S
    src/chimera/fix/camera_shake_fix.cpp
    src/chimera/fix/camera_shake_fix.S
    src/chimera/fix/checkpoint_fix.cpp
    src/chimera/fix/checkpoint_fix.S
    src/chimera/fix/chicago_fix.cpp
    src/chimera/fix/chicago_fix.S
    src/chimera/fix/contrail_fix.cpp
    src/chimera/fix/contrail_fix.S
    src/chimera/fix/custom_map_lobby_fix.cpp
    src/chimera/fix/death_reset_time.cpp
    src/chimera/fix/z_fighting.cpp
    src/chimera/fix/z_fighting.S
    src/chimera/fix/descope_fix.cpp
    src/chimera/fix/extend_limits.cpp
    src/chimera/fix/extend_limits.S
    src/chimera/fix/extended_description_fix.cpp
    src/chimera/fix/map_hacks/map_hacks.cpp
    src/chimera/fix/floor_decal_memery.cpp
    src/chimera/fix/floor_decal_memery.S
    src/chimera/fix/force_crash.cpp
    src/chimera/fix/fp_model.cpp
    src/chimera/fix/fp_model.S
    src/chimera/fix/fp_reverb.cpp
    src/chimera/fix/fp_reverb.S
    src/chimera/fix/fov_fix.cpp
    src/chimera/fix/gametype_indicator_memes.cpp
    src/chimera/fix/gametype_indicator_memes.S
    src/chimera/fix/hud_bitmap_scale.cpp
    src/chimera/fix/hud_bitmap_scale.S
    src/chimera/fix/hud_meters.cpp
    src/chimera/fix/hud_meters.S
    src/chimera/fix/invalid_command_crash.cpp
    src/chimera/fix/invalid_command_crash.S
    src/chimera/fix/interpolate/antenna.cpp
    src/chimera/fix/interpolate/camera.cpp
    src/chimera/fix/interpolate/flag.cpp
    src/chimera/fix/interpolate/fp.cpp
    src/chimera/fix/interpolate/interpolate.cpp
    src/chimera/fix/interpolate/light.cpp
    src/chimera/fix/interpolate/object.cpp
    src/chimera/fix/interpolate/particle.cpp
    src/chimera/fix/jason_jones_hacks.cpp
    src/chimera/fix/leak_descriptors.cpp
    src/chimera/fix/model_detail.cpp
    src/chimera/fix/model_detail.S
    src/chimera/fix/motion_sensor_fix.cpp
    src/chimera/fix/motion_sensor_fix.S
    src/chimera/fix/multitexture_overlay_fix.cpp
    src/chimera/fix/multitexture_overlay_fix.S
    src/chimera/fix/name_fade.cpp
    src/chimera/fix/name_fade.S
    src/chimera/fix/pas_crash_fix.cpp
    src/chimera/fix/pas_crash_fix.S
    src/chimera/fix/glass_fix.cpp
    src/chimera/fix/glass_fix.S
    src/chimera/fix/internal_shaders.cpp
    src/chimera/fix/internal_shaders.S
    src/chimera/fix/sane_defaults.cpp
    src/chimera/fix/sane_defaults.S
    src/chimera/fix/scoreboard_fade_fix.cpp
    src/chimera/fix/scoreboard_fade_fix.S
    src/chimera/fix/screen_effect_fix.cpp
    src/chimera/fix/screen_effect_fix.S
    src/chimera/fix/specular_memes.cpp
    src/chimera/fix/specular_memes.S
    src/chimera/fix/sun_fix.cpp
    src/chimera/fix/timer_offset.cpp
    src/chimera/fix/uncompressed_sound_fix.cpp
    src/chimera/fix/vehicle_team_desync.cpp
    src/chimera/fix/vehicle_team_desync.S
    src/chimera/fix/video_mode.cpp
    src/chimera/fix/video_mode.S
    src/chimera/fix/water_fix.cpp
    src/chimera/fix/water_fix.S
    src/chimera/fix/weapon_swap_ticks.cpp
    src/chimera/fix/weapon_swap_ticks.S
    src/chimera/fix/weather_fix.cpp
    src/chimera/fix/weather_fix.S
    src/chimera/fix/widescreen_element_reposition_hud.S
    src/chimera/fix/widescreen_element_reposition_letterbox.S
    src/chimera/fix/widescreen_element_reposition_menu.S
    src/chimera/fix/widescreen_element_reposition_menu_text.S
    src/chimera/fix/widescreen_fix.cpp
    src/chimera/fix/widescreen_mouse.S
    src/chimera/fix/xbox_channel_order.cpp
    src/chimera/fix/xbox_channel_order.S
    src/chimera/fix/biped_ui_spawn.cpp
    src/chimera/fix/effect_shader_fix.cpp
    src/chimera/fix/effect_shader_fix.S
    src/chimera/halo_data/antenna.cpp
    src/chimera/halo_data/bitmaps.cpp
    src/chimera/halo_data/camera.cpp
    src/chimera/halo_data/chat.cpp
    src/chimera/halo_data/chat.S
    src/chimera/halo_data/contrail.cpp
    src/chimera/halo_data/controls.cpp
    src/chimera/halo_data/cutscene.cpp
    src/chimera/halo_data/damage.cpp
    src/chimera/halo_data/damage.S
    src/chimera/halo_data/decal.cpp
    src/chimera/halo_data/effect.cpp
    src/chimera/halo_data/flag.cpp
    src/chimera/halo_data/game_engine.cpp
    src/chimera/halo_data/game_functions.cpp
    src/chimera/halo_data/game_functions.S
    src/chimera/halo_data/game_variables.cpp
    src/chimera/halo_data/globals.cpp
    src/chimera/halo_data/hud_fonts.cpp
    src/chimera/halo_data/hud_fonts.S
    src/chimera/halo_data/keyboard.cpp
    src/chimera/halo_data/light.cpp
    src/chimera/halo_data/map.cpp
    src/chimera/halo_data/map.S
    src/chimera/halo_data/menu.cpp
    src/chimera/halo_data/menu.S
    src/chimera/halo_data/main_menu_music.cpp
    src/chimera/halo_data/multiplayer.cpp
    src/chimera/halo_data/object.cpp
    src/chimera/halo_data/object.S
    src/chimera/halo_data/particle.cpp
    src/chimera/halo_data/path.cpp
    src/chimera/halo_data/pause.cpp
    src/chimera/halo_data/player.cpp
    src/chimera/halo_data/port.cpp
    src/chimera/halo_data/resolution.cpp
    src/chimera/halo_data/script.cpp
    src/chimera/halo_data/script.S
    src/chimera/halo_data/server.cpp
    src/chimera/halo_data/tag.cpp
    src/chimera/halo_data/tag_class.cpp
    src/chimera/localization/localization.cpp
    src/chimera/lua/lua_callback.cpp
    src/chimera/lua/lua_filesystem.cpp
    src/chimera/lua/lua_game.cpp
    src/chimera/lua/lua_io.cpp
    src/chimera/lua/lua_script.cpp
    src/chimera/lua/lua_variables.cpp
    src/chimera/lua/scripting.cpp
    src/chimera/map_loading/compression.cpp
    src/chimera/map_loading/map_loading.cpp
    src/chimera/map_loading/map_loading.S
    src/chimera/master_server/master_server.cpp
    src/chimera/math_trig/math_trig.cpp
    src/chimera/miscellaneous/controller.cpp
    src/chimera/miscellaneous/controller.S
    src/chimera/output/draw_text.cpp
    src/chimera/output/draw_text.S
    src/chimera/output/error_box.cpp
    src/chimera/output/output.cpp
    src/chimera/output/output.S
    src/chimera/rasterizer/shader_transparent_generic.cpp
    src/chimera/rasterizer/shader_transparent_generic.S
    src/chimera/rasterizer/rasterizer_transparent_geometry.cpp
    src/chimera/rasterizer/rasterizer.cpp
    src/chimera/rasterizer/rasterizer_vertex_shaders.cpp
    src/chimera/signature/hook.cpp
    src/chimera/signature/signature.cpp
    src/chimera/signature/hac/codefinder.cpp
    src/chimera/version.rc
    ${COMMAND_FILES}

    ${CMAKE_CURRENT_BINARY_DIR}/localization_strings.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/color_codes.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/vertex_shaders.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pixel_shaders.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/d3dx_effects.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/effects_collection.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_generic_blobs.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/map_hacks_config.cpp
)
add_dependencies(chimera chimera-version local_curl local_zstd)

# Set how we'll generate localization_string
file(GLOB CHIMERA_LOCALIZATION_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/localization/language/*")
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/localization_strings.hpp
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/localization/localizer.py ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/localization/language ${CMAKE_CURRENT_BINARY_DIR}/localization_strings.hpp
    DEPENDS ${CHIMERA_LOCALIZATION_DEPS}
)

# Set how we'll generate color_codes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/color_codes.hpp
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/output/color_codes_generator.py ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/output/color_codes ${CMAKE_CURRENT_BINARY_DIR}/color_codes.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/output/color_codes
)

file(GLOB CHIMERA_SHADER_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/halo_data/shaders/*")
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/vertex_shaders.cpp" "${CMAKE_CURRENT_BINARY_DIR}/pixel_shaders.cpp" "${CMAKE_CURRENT_BINARY_DIR}/d3dx_effects.cpp" "${CMAKE_CURRENT_BINARY_DIR}/effects_collection.cpp"
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/halo_data/shaders/generate_shader_blobs.py ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/halo_data/shaders/ ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS ${CHIMERA_SHADER_DEPS}
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_generic_blobs.cpp"
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/halo_data/shaders/generate_shader_transparent_generic_blobs.py ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/halo_data/shaders/ ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/halo_data/shaders/pixel/generic_hashes.txt
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/map_hacks_config.cpp"
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/fix/map_hacks/map_hacks_config_generate.py ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/fix/map_hacks/ ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/fix/map_hacks/map_hacks_config.json
)

target_include_directories(chimera
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${LOCAL_CURL_INCLUDE_DIR}
    PRIVATE ${LOCAL_ZSTD_INCLUDE_DIR}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/lua"
)

# Set the name
target_link_libraries(chimera ${D3DCOMPILER_LIBRARY} shlwapi blake3 map_downloader lua ${LOCAL_CURL_LIBRARIES} local_zstd)

# This one isn't worth fixing
set_source_files_properties(src/chimera/signature/hac/codefinder.cpp PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")

# Set character encoding
set_source_files_properties(src/chimera/localization/localization.cpp PROPERTIES COMPILE_FLAGS "-fexec-charset=iso-8859-1")

# Ignore this warning
set_source_files_properties(src/chimera/custom_chat/custom_chat.cpp PROPERTIES COMPILE_FLAGS "-Wno-format")
