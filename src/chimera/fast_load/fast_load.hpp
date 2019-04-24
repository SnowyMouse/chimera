#ifndef CHIMERA_FAST_LOAD_HPP
#define CHIMERA_FAST_LOAD_HPP

namespace Chimera {
    /**
     * Initialize fast loading
     */
    void initialize_fast_load() noexcept;

    /**
     * Get the path for the map
     * @param  map name of the map
     * @return     path to the map if found
     */
    const char *path_for_map(const char *map) noexcept;
}

#endif
