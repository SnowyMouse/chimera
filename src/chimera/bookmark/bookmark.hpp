// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_BOOKMARK_HPP
#define CHIMERA_BOOKMARK_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

namespace Chimera {
    struct Bookmark {
        char address[64];
        std::uint16_t port;
        char password[9];
        bool brackets;
    };

    struct QueryPacketDone {
        enum Error {
            NONE = 0,
            FAILED_TO_RESOLVE,
            TIMED_OUT
        };

        Bookmark b;
        std::unordered_map<std::string, std::string> query_data;
        bool timed_out;
        Error error;
        std::size_t ping = 0;

        const char *get_data_for_key(const char *key) {
            for(auto &q : this->query_data) {
                if(q.first == key) {
                    return q.second.data();
                }
            }
            return nullptr;
        }
    };

    /**
     * Query server?
     * @param  what server to query
     * @return      packet
     */
    QueryPacketDone query_server(const Bookmark &what);

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
