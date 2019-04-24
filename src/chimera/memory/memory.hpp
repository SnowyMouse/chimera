#ifndef CHIMERA_MEMORY_HPP
#define CHIMERA_MEMORY_HPP

#include <cstdint>
#include <cstddef>

#define CHIMERA_MEMORY_ALLOCATION_SIZE 0x40000000

namespace Chimera {
    /**
     * Set up Chimera's memory allocation
     */
    void set_up_chimera_memory() noexcept;

    /**
     * Free Chimera's memory allocation
     */
    void free_chimera_memory() noexcept;

    /**
     * Get the memory
     * @return the memory
     */
    std::byte *get_memory() noexcept;

    /**
     * Read memory
     * @param output output to write data to
     * @param offset offset of memory to read
     * @param size   size of memory to read
     */
    void read_memory(std::byte *output, std::size_t offset, std::size_t size) noexcept;

    /**
     * Write memory
     * @param input  input to read data from
     * @param offset offset of memory to write
     * @param size   size of memory to write
     */
    void write_memory(const std::byte *input, std::size_t offset, std::size_t size) noexcept;

    /**
     * Read the map at the path
     * @param  path path to read from
     */
    void read_map(const char *path);
}

#endif
