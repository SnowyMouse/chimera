# Build our own ZSTD and cURL

# Silence CMake Warning
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(ExternalProject)

# This was once OFF by default, but newer CMake versions changed this to ON.
# As long as the hashes are the same it does not matter. Needed for some MinGW-w64 releases.
set(CMAKE_TLS_VERIFY OFF)

# curl versions past 8.17.0 dropped support for Windows XP using the older MSVCRT C runtime.
# 8.18.0 will work with UCRT and is assumed to be the last version to support Windows XP at all.
if(${CHIMERA_WINXP})
    set(LOCAL_CURL_URL "https://github.com/curl/curl/releases/download/curl-8_17_0/curl-8.17.0.tar.gz")
    set(LOCAL_CURL_URL_HASH e8e74cdeefe5fb78b3ae6e90cd542babf788fa9480029cfcee6fd9ced42b7910)
else()
    set(LOCAL_CURL_URL "https://github.com/curl/curl/releases/download/curl-8_18_0/curl-8.18.0.tar.gz")
    set(LOCAL_CURL_URL_HASH e9274a5f8ab5271c0e0e6762d2fce194d5f98acc568e4ce816845b2dcc0cf88f)
endif()

set(LOCAL_CURL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/curl/include)
set(LOCAL_CURL_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/curl/lib)

ExternalProject_Add(curl
    PREFIX ext/curl
    URL ${LOCAL_CURL_URL}
    URL_HASH SHA256=${LOCAL_CURL_URL_HASH}
    BUILD_BYPRODUCTS ${LOCAL_CURL_LIB_DIR}/libcurl.a
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_INSTALL_INCLUDEDIR=include
        -DCMAKE_INSTALL_LIBDIR=lib
        -DBUILD_SHARED_LIBS=OFF
        -DBUILD_STATIC_LIBS=ON
        -DBUILD_LIBCURL_DOCS=OFF
        -DBUILD_CURL_EXE=OFF
        -DCURL_BROTLI=OFF
        -DCURL_USE_LIBPSL=OFF
        -DCURL_USE_LIBSSH2=OFF
        -DCURL_ZLIB=OFF
        -DCURL_ZSTD=OFF
        -DHTTP_ONLY=ON
        -DUSE_NGHTTP2=OFF
        -DUSE_LIBIDN2=OFF
        -DCURL_TARGET_WINDOWS_VERSION=${CHIMERA_TARGET_WINDOWS_VERSION}
)

add_library(local_curl STATIC IMPORTED)
set_target_properties(local_curl PROPERTIES IMPORTED_LOCATION ${LOCAL_CURL_LIB_DIR}/libcurl.a)
add_dependencies(local_curl curl)

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

if(${CHIMERA_WINXP})
    set(LOCAL_CURL_LIBRARIES local_curl ws2_32 bcrypt)
else()
    set(LOCAL_CURL_LIBRARIES local_curl ws2_32 bcrypt iphlpapi)
endif()
