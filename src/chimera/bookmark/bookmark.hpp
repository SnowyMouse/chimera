// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_BOOKMARK_HPP
#define CHIMERA_BOOKMARK_HPP

#include <string>
#include <vector>
#include <utility>

namespace Chimera {
    struct Bookmark {
        char address[64];
        std::uint16_t port;
        char password[9];
        bool brackets;
    };

    /**
     * Get the latest connection made
     * @return reference to the latest connection
     */
    const Bookmark &get_latest_connection() noexcept;

    /**
     * Set up server history
     */
    void set_up_server_history() noexcept;

    /**
     * Load the bookmarks file
     * @param file file to load from
     * @return     array of bookmarks and the server names
     */
    std::vector<Bookmark> load_bookmarks_file(const char *file) noexcept;

    /**
     * Save the bookmarks file
     * @param file      file to save to
     * @param bookmarks bookmarks to save
     */
    void save_bookmarks_file(const char *file, const std::vector<Bookmark> &bookmarks) noexcept;
}

#endif
