# SPDX-License-Identifier: GPL-3.0-only

include(src/chimera/command/command.cmake)

# Include version script
add_custom_command(
    OUTPUT src/chimera/version.hpp
    COMMAND "${CMAKE_COMMAND}" "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}" "-DGIT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/.git" "-DOUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/version.hpp" -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DIN_GIT_REPO=${IN_GIT_REPO} -P ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/version.cmake
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/refs/heads/${CHIMERA_GIT_BRANCH}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/version.cmake"
)

# Make chimera-version
add_custom_target(chimera-version
    SOURCES src/chimera/version.hpp
)

# Define the source files
add_library(chimera STATIC
    src/chimera/map_loading/laa.cpp
    src/chimera/annoyance/auto_get_list.cpp
    src/chimera/annoyance/auto_get_list.S
    src/chimera/annoyance/drm.cpp
    src/chimera/annoyance/drm.S
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
    src/chimera/event/map_load.cpp
    src/chimera/event/rcon_message.cpp
    src/chimera/event/tick.cpp
    src/chimera/fast_load/crc32.c
    src/chimera/fast_load/fast_load.cpp
    src/chimera/fast_load/fast_load.S
    src/chimera/fix/abolish_safe_mode.cpp
    src/chimera/fix/aim_assist.cpp
    src/chimera/fix/aim_assist.S
    src/chimera/fix/auto_center.cpp
    src/chimera/fix/blue_32bit_color_fix.cpp
    src/chimera/fix/bullshit_server_data.cpp
    src/chimera/fix/bullshit_server_data.S
    src/chimera/fix/camera_shake_fix.cpp
    src/chimera/fix/camera_shake_fix.S
    src/chimera/fix/contrail_fix.cpp
    src/chimera/fix/contrail_fix.S
    src/chimera/fix/custom_edition_bridge/map_support.cpp
    src/chimera/fix/custom_edition_bridge/map_support.S
    src/chimera/fix/custom_edition_bridge/netcode.cpp
    src/chimera/fix/custom_edition_bridge/netcode.S
    src/chimera/fix/custom_map_lobby_fix.cpp
    src/chimera/fix/death_reset_time.cpp
    src/chimera/fix/descope_fix.cpp
    src/chimera/fix/extend_limits.cpp
    src/chimera/fix/extend_limits.S
    src/chimera/fix/extended_description_fix.cpp
    src/chimera/fix/floor_decal_memery.cpp
    src/chimera/fix/floor_decal_memery.S
    src/chimera/fix/force_crash.cpp
    src/chimera/fix/fp_reverb.cpp
    src/chimera/fix/fp_reverb.S
    src/chimera/fix/fov_fix.cpp
    src/chimera/fix/invalid_command_crash.cpp
    src/chimera/fix/invalid_command_crash.S
    src/chimera/fix/inverted_flag.cpp
    src/chimera/fix/inverted_flag.S
    src/chimera/fix/interpolate/antenna.cpp
    src/chimera/fix/interpolate/camera.cpp
    src/chimera/fix/interpolate/flag.cpp
    src/chimera/fix/interpolate/fp.cpp
    src/chimera/fix/interpolate/interpolate.cpp
    src/chimera/fix/interpolate/light.cpp
    src/chimera/fix/interpolate/object.cpp
    src/chimera/fix/interpolate/particle.cpp
    src/chimera/fix/leak_descriptors.cpp
    src/chimera/fix/model_detail.cpp
    src/chimera/fix/model_detail.S
    src/chimera/fix/motion_sensor_fix.cpp
    src/chimera/fix/motion_sensor_fix.S
    src/chimera/fix/name_fade.cpp
    src/chimera/fix/name_fade.S
    src/chimera/fix/nav_numbers.cpp
    src/chimera/fix/sane_defaults.cpp
    src/chimera/fix/sane_defaults.S
    src/chimera/fix/scoreboard_fade_fix.cpp
    src/chimera/fix/scoreboard_fade_fix.S
    src/chimera/fix/sun_fix.cpp
    src/chimera/fix/timer_offset.cpp
    src/chimera/fix/uncompressed_sound_fix.cpp
    src/chimera/fix/vehicle_team_desync.cpp
    src/chimera/fix/vehicle_team_desync.S
    src/chimera/fix/video_mode.cpp
    src/chimera/fix/video_mode.S
    src/chimera/fix/weapon_swap_ticks.cpp
    src/chimera/fix/weapon_swap_ticks.S
    src/chimera/fix/widescreen_element_reposition_hud.S
    src/chimera/fix/widescreen_element_reposition_letterbox.S
    src/chimera/fix/widescreen_element_reposition_menu.S
    src/chimera/fix/widescreen_element_reposition_menu_text.S
    src/chimera/fix/widescreen_fix.cpp
    src/chimera/fix/widescreen_mouse.S
    src/chimera/halo_data/antenna.cpp
    src/chimera/halo_data/camera.cpp
    src/chimera/halo_data/chat.cpp
    src/chimera/halo_data/chat.S
    src/chimera/halo_data/controls.cpp
    src/chimera/halo_data/damage.cpp
    src/chimera/halo_data/damage.S
    src/chimera/halo_data/decal.cpp
    src/chimera/halo_data/effect.cpp
    src/chimera/halo_data/flag.cpp
    src/chimera/halo_data/game_engine.cpp
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
    src/chimera/halo_data/spawn_object.cpp
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
    src/chimera/lua/lua_game.cpp
    src/chimera/lua/lua_io.cpp
    src/chimera/lua/lua.cpp
    src/chimera/map_loading/map_loading.cpp
    src/chimera/map_loading/map_loading.S
    src/chimera/master_server/master_server.cpp
    src/chimera/math_trig/math_trig.cpp
    src/chimera/miscellaneous/controller.cpp
    src/chimera/miscellaneous/controller.S
    src/chimera/output/draw_text.cpp
    src/chimera/output/draw_text.S
    src/chimera/output/output.cpp
    src/chimera/output/output.S
    src/chimera/signature/hook.cpp
    src/chimera/signature/signature.cpp
    src/chimera/signature/hac/codefinder.cpp
    src/chimera/version.rc
    ${COMMAND_FILES}

    ${CMAKE_CURRENT_BINARY_DIR}/localization_strings.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/color_codes.hpp
)
add_dependencies(chimera chimera-version)

# Set how we'll generate localization_string
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/localization_strings.hpp
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/localization/localizer.py ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/localization/language ${CMAKE_CURRENT_BINARY_DIR}/localization_strings.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/localization/language/*
)

# Set how we'll generate color_codes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/color_codes.hpp
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/output/color_codes_generator.py ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/output/color_codes ${CMAKE_CURRENT_BINARY_DIR}/color_codes.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/chimera/output/color_codes
)

target_include_directories(chimera
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ext/curl/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ext/zstd/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/lua"
)

# Set the name
target_link_libraries(chimera shlwapi hac_map_downloader lua ${CMAKE_CURRENT_SOURCE_DIR}/ext/curl/lib/libcurl.a ws2_32)

# Target this
target_include_directories(chimera PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ext/zstd/include)

# This one isn't worth fixing
set_source_files_properties(src/chimera/signature/hac/codefinder.cpp PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")

# Set character encoding
set_source_files_properties(src/chimera/localization/localization.cpp PROPERTIES COMPILE_FLAGS "-fexec-charset=iso-8859-1")

# Ignore this warning
set_source_files_properties(src/chimera/custom_chat/custom_chat.cpp PROPERTIES COMPILE_FLAGS "-Wno-format")

option(CHIMERA_DISABLE_CUSTOM_EDITION_FIXES "Limit detail over reflection and fog fixes to retail/demo" OFF)
if(${CHIMERA_DISABLE_CUSTOM_EDITION_FIXES})
    add_definitions(-DCHIMERA_DISABLE_CUSTOM_EDITION_FIXES)
endif()
