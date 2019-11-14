#include "bookmark.hpp"
#include "../chimera.hpp"
#include <windows.h>
#include <cstring>
#include <fstream>
#include "../event/connect.hpp"
#include "../output/output.hpp"

namespace Chimera {
    #define MAX_HISTORY_SIZE 20

    static Bookmark latest_connection = {};

    const Bookmark &get_latest_connection() noexcept {
        return latest_connection;
    }

    static bool on_connect(std::uint32_t &ip, std::uint16_t &port, const char *password) {
        // Prepare the bookmark
        std::uint8_t *ip_chars = reinterpret_cast<std::uint8_t *>(&ip);
        Bookmark x = {};
        std::snprintf(x.ip_address, sizeof(x.ip_address), "%i.%i.%i.%i:%i", ip_chars[3], ip_chars[2], ip_chars[1], ip_chars[0], port);
        std::snprintf(x.password, sizeof(x.password), "%s", password);

        // See if it's already in the history. If so, remove it
        auto history = load_bookmarks_file("history.txt");
        for(auto &h : history) {
            if(std::strcmp(h.ip_address, x.ip_address) == 0) {
                history.erase(history.begin() + (&h - history.data()));
                break;
            }
        }

        // Add it to the front
        history.insert(history.begin(), x);

        // If we have too many items in the history, remove the last one
        while(history.size() > MAX_HISTORY_SIZE) {
            history.erase(history.end() - 1);
        }

        // Save
        save_bookmarks_file("history.txt", history);

        // Keep this for later
        latest_connection = x;

        return true;
    }

    void set_up_server_history() noexcept {
        add_preconnect_event(on_connect, EVENT_PRIORITY_FINAL);
    }

    std::vector<Bookmark> load_bookmarks_file(const char *file) noexcept {
        char path[MAX_PATH];
        std::snprintf(path, sizeof(path), "%s\\%s", get_chimera().get_path(), file);
        std::ifstream f(path);
        std::string line;
        std::vector<Bookmark> bookmarks;
        while(std::getline(f, line)) {
            Bookmark b;
            std::size_t i;
            for(i = 0; i < sizeof(b.ip_address) - 1 && i < line.size() && line[i] != ' ' && line[i] != '\r' && line[i] != '\n'; i++) {
                b.ip_address[i] = line[i];
            }
            b.ip_address[i] = 0;
            if(line[i++] == ' ') {
                std::size_t s;
                for(s = 0; s < sizeof(b.server_name) - 1 && i < line.size() && line[i] != '\r' && line[i] != '\n'; i++, s++) {
                    b.server_name[s] = line[i];
                }
                b.server_name[s] = 0;
            }
            bookmarks.push_back(b);
        }
        return bookmarks;
    }

    void save_bookmarks_file(const char *file, const std::vector<Bookmark> &bookmarks) noexcept {
        char path[MAX_PATH];
        std::snprintf(path, sizeof(path), "%s\\%s", get_chimera().get_path(), file);
        std::ofstream f(path, std::ios_base::out | std::ios_base::trunc);

        for(auto &b : bookmarks) {
            char line[256];
            std::snprintf(line, sizeof(line), "%s %s", b.ip_address, b.server_name);
            for(auto &c : line) {
                if(c == '\r' || c == '\n' || c == '\t') {
                    c = ' ';
                }
            }

            f << line << "\n";
        }

        f.flush();
        f.close();
    }
}
