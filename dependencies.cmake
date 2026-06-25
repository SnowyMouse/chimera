# Build our own ZSTD and cURL

# Silence CMake Warning
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(ExternalProject)

# This was once OFF by default, but newer CMake versions changed this to ON.
# As long as the hashes are the same it should be fine. Needed for some MinGW-w64 releases, mainly winlibs.
set(CMAKE_TLS_VERIFY OFF)

if(${CHIMERA_WINXP})
	if(NOT ${CHIMERA_WINXP_UCRT})
		# curl 8.17.0 is the last version to support Windows XP using the older MSVCRT C runtime.
		set(LOCAL_CURL_URL "https://github.com/curl/curl/releases/download/curl-8_17_0/curl-8.17.0.tar.xz")
		set(LOCAL_CURL_URL_HASH 955f6e729ad6b3566260e8fef68620e76ba3c31acf0a18524416a185acf77992)
	else()
		# curl 8.18.0 will only work on Windows XP with the UCRT C runtime and is the last version to support XP at all.
		set(LOCAL_CURL_URL "https://github.com/curl/curl/releases/download/curl-8_18_0/curl-8.18.0.tar.xz")
		set(LOCAL_CURL_URL_HASH 40df79166e74aa20149365e11ee4c798a46ad57c34e4f68fd13100e2c9a91946)
	endif()
else()
	# use latest version
    set(LOCAL_CURL_URL "https://github.com/curl/curl/releases/download/curl-8_21_0/curl-8.21.0.tar.xz")
    set(LOCAL_CURL_URL_HASH aa1b66a70eace83dc624508745646c08ae561de512ab403adffb93ac87fc72e6)
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
