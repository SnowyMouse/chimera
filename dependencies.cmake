# Build our own ZSTD and cURL

# Silence CMake Warning
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(ExternalProject)

set(LOCAL_CURL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/curl/include)
set(LOCAL_CURL_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/curl/lib)

ExternalProject_Add(curl
    PREFIX ext/curl
    URL "https://github.com/curl/curl/releases/download/curl-8_11_1/curl-8.11.1.tar.gz"
    URL_HASH SHA256=a889ac9dbba3644271bd9d1302b5c22a088893719b72be3487bc3d401e5c4e80
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
)

add_library(local_curl STATIC IMPORTED)
set_target_properties(local_curl PROPERTIES IMPORTED_LOCATION ${LOCAL_CURL_LIB_DIR}/libcurl.a)
add_dependencies(local_curl curl)

set(LOCAL_ZSTD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/zstd/include)
set(LOCAL_ZSTD_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/zstd/lib)

ExternalProject_Add(zstd
    PREFIX ext/zstd
    URL "https://github.com/facebook/zstd/releases/download/v1.5.6/zstd-1.5.6.tar.gz"
    URL_HASH SHA256=8c29e06cf42aacc1eafc4077ae2ec6c6fcb96a626157e0593d5e82a34fd403c1
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
