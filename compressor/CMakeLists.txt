# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.22.1)

project("chimera-compress")

option(BUILD_STATIC_ZSTD "Build and use static Zstandard as part of this project" OFF)

if(BUILD_STATIC_ZSTD)
    # Silence CMake Warning
    if (POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()

    include(ExternalProject)

    set(LOCAL_ZSTD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/zstd/include)
    set(LOCAL_ZSTD_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/zstd/lib)

    ExternalProject_Add(zstd
        PREFIX ext/zstd
        URL "https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz"
        URL_HASH SHA256=eb33e51f49a15e023950cd7825ca74a4a2b43db8354825ac24fc1b7ee09e6fa3
        BUILD_BYPRODUCTS ${LOCAL_ZSTD_LIB_DIR}/libzstd.a
        SOURCE_SUBDIR build/cmake
        CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_INSTALL_INCLUDEDIR=include
            -DCMAKE_INSTALL_LIBDIR=lib
            -DBUILD_SHARED_LIBS=OFF
            -DZSTD_BUILD_CONTRIB=OFF
            -DZSTD_BUILD_STATIC=ON
            -DZSTD_BUILD_SHARED=OFF
            -DZSTD_BUILD_PROGRAMS=OFF
            -DZSTD_BUILD_TESTS=OFF
    )

    add_library(local_zstd STATIC IMPORTED)
    set_target_properties(local_zstd PROPERTIES IMPORTED_LOCATION ${LOCAL_ZSTD_LIB_DIR}/libzstd.a)
    add_dependencies(local_zstd zstd)
endif()

add_executable(chimera-compress
    main.cpp
)

if(WIN32)
    target_sources(chimera-compress
        PRIVATE version.rc
    )
endif()

if(BUILD_STATIC_ZSTD)
    add_dependencies(chimera-compress local_zstd)
    target_include_directories(chimera-compress PRIVATE ${LOCAL_ZSTD_INCLUDE_DIR})
    set(ZSTD_LIB local_zstd)
    if(WIN32)
        set_target_properties(chimera-compress PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++ -static -lwinpthread")
    endif()
else()
    set(ZSTD_LIB zstd)
endif()

set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS "-Wno-multichar")

target_link_libraries(chimera-compress ${ZSTD_LIB})
