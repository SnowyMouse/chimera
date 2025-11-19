# SPDX-License-Identifier: GPL-3.0-only

add_library(map_downloader STATIC
    src/map_downloader/map_downloader.cpp
)

add_dependencies(map_downloader local_curl)

# Target this
target_include_directories(map_downloader PRIVATE ${LOCAL_CURL_INCLUDE_DIR})

add_executable(hac_map_downloader_test
    src/map_downloader/test/test.cpp
)

add_dependencies(hac_map_downloader_test map_downloader)
target_link_libraries(hac_map_downloader_test map_downloader ${LOCAL_CURL_LIBRARIES})
set_target_properties(hac_map_downloader_test PROPERTIES LINK_FLAGS "-m32 -static-libgcc -static-libstdc++ -static -lwinpthread")
