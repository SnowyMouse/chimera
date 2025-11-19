# SPDX-License-Identifier: GPL-3.0-only

# Define the source files
add_library(monolith MODULE
    src/monolith/monolith.c
    src/monolith/strings/strings.rc
)

target_include_directories(monolith
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

set_property(SOURCE src/monolith/strings/strings.rc
             APPEND PROPERTY OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/version.hpp")

# Set the name
set_target_properties(monolith PROPERTIES PREFIX "")
set_target_properties(monolith PROPERTIES OUTPUT_NAME "strings")

# lol
add_dependencies(monolith chimera)
set_target_properties(monolith PROPERTIES LINK_FLAGS "-m32 -static-libgcc -static-libstdc++ -static -lwinpthread")
target_link_libraries(monolith chimera ${WINXP_COMPATIBILITY_LIBRARIES} shlwapi version map_downloader ${LOCAL_CURL_LIBRARIES} local_zstd)
