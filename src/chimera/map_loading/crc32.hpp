#ifndef CRC32_HPP
#define CRC32_HPP

#include <cstdint>

extern "C" std::uint32_t crc32(std::uint32_t crc, const void *buf, std::size_t size);

#endif