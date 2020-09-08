// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include <ws2tcpip.h>
#include "bookmark.hpp"
#include "../chimera.hpp"
#include "../event/frame.hpp"
#include <windows.h>
#include <cstring>
#include <optional>
#include <fstream>
#include "../event/connect.hpp"
#include "../output/output.hpp"
#include "../halo_data/script.hpp"
#include "../halo_data/resolution.hpp"
#include "../localization/localization.hpp"
#include <mutex>
#include <thread>

namespace Chimera {
    #define MAX_HISTORY_SIZE 20

    static Bookmark latest_connection = {};

    const Bookmark &get_latest_connection() noexcept {
        return latest_connection;
    }

    static std::optional<Bookmark> parse_bookmark(const char *line) {
        Bookmark b = {};
        std::size_t line_length = std::strlen(line);

        // Get the address
        std::size_t i;
        std::string address;
        std::size_t address_length = 0;
        for(i = 0; i < sizeof(b.address) - 1 && i < line_length && line[i] != '\r' && line[i] != '\n' && line[i] != ':' && line[i] != ' '; i++, address_length++);
        if(address_length < 2) {
            return std::nullopt;
        }
        else if(line[0] == '[' && line[address_length - 1] == ']') {
            address = std::string(line + 1, address_length - 1);
            b.brackets = true;
        }
        else {
            address = std::string(line, address_length);
        }
        std::strncpy(b.address, address.data(), sizeof(b.address) - 1);

        // Now the port
        if(line[i++] == ':') {
            // Increment port_length until we get to the end
            std::size_t port_length;
            for(port_length = 0; i < line_length && line[i] != '\r' && line[i] != '\n' && line[i] != ' '; i++, port_length++);

            // Attempt to convert to port
            try {
                b.port = static_cast<std::uint16_t>(std::stoi(std::string(line + address_length + 1, port_length)));
            }
            catch(std::exception &) {
                return std::nullopt;
            }
        }

        // Default to 2302 otherwise
        else {
            b.port = 2302;
            i--;
        }

        // Lastly, the password (if present)
        if(line[i++] == ' ') {
            std::size_t s;
            for(s = 0; s < sizeof(b.password) - 1 && i < line_length && line[i] != '\r' && line[i] != '\n'; i++, s++) {
                b.password[s] = line[i];
            }
            b.password[s] = 0;
        }

        return b;
    }

    static bool on_connect(std::uint32_t &ip, std::uint16_t &port, const char *password) {
        // Prepare the bookmark
        std::uint8_t *ip_chars = reinterpret_cast<std::uint8_t *>(&ip);
        Bookmark x = {};
        std::snprintf(x.address, sizeof(x.address), "%i.%i.%i.%i", ip_chars[3], ip_chars[2], ip_chars[1], ip_chars[0]);
        x.port = port;
        std::snprintf(x.password, sizeof(x.password), "%s", password);

        // See if it's already in the history. If so, remove it
        auto history = load_bookmarks_file("history.txt");
        for(auto &h : history) {
            if(std::strcmp(h.address, x.address) == 0 && x.port == h.port) {
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
            auto parsed_bookmark = parse_bookmark(line.data());
            if(parsed_bookmark.has_value()) {
                bookmarks.push_back(*parsed_bookmark);
            }
        }
        return bookmarks;
    }

    void save_bookmarks_file(const char *file, const std::vector<Bookmark> &bookmarks) noexcept {
        char path[MAX_PATH];
        std::snprintf(path, sizeof(path), "%s\\%s", get_chimera().get_path(), file);
        std::ofstream f(path, std::ios_base::out | std::ios_base::trunc);

        for(auto &b : bookmarks) {
            char line[256];

            // If we have a password, also include that.
            if(b.password[0]) {
                std::snprintf(line, sizeof(line), "%s%s%s:%u %s", b.brackets ? "[" : "", b.address, b.brackets ? "]" : "", b.port, b.password);
            }
            // Otherwise it's not really required to have a password
            else {
                std::snprintf(line, sizeof(line), "%s%s%s:%u", b.brackets ? "[" : "", b.address, b.brackets ? "]" : "", b.port);
            }
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
    static std::vector<QueryPacketDone> finished_packets;

    static std::mutex querying;

    QueryPacketDone query_server(const Bookmark &what) {
        QueryPacketDone finished_packet;
        finished_packet.b = what;
        struct addrinfo *address;

        // Lookup it
        char port[6] = {};
        std::snprintf(port, sizeof(port), "%u", what.port);
        int q = getaddrinfo(what.address, port, nullptr, &address);
        if(q != 0) {
            finished_packet.error = QueryPacketDone::Error::FAILED_TO_RESOLVE;
            return finished_packet;
        }
        auto family = address->ai_family;
        if(family != AF_INET && family != AF_INET6) {
            freeaddrinfo(address);
            finished_packet.error = QueryPacketDone::Error::FAILED_TO_RESOLVE;
            return finished_packet;
        }

        // Do the thing
        struct sockaddr_storage saddr = {};
        std::size_t saddr_size = address->ai_addrlen;

        // Free it all
        std::memcpy(&saddr, address->ai_addr, saddr_size);
        freeaddrinfo(address);

        // Do socket things
        SOCKET s = socket(family, SOCK_DGRAM, IPPROTO_UDP);
        DWORD opt = 700;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&opt), sizeof(opt));
        static constexpr char PACKET_QUERY[] = "\\query";
        q = sendto(s, PACKET_QUERY, sizeof(PACKET_QUERY) - 1, 0, reinterpret_cast<sockaddr *>(&saddr), saddr_size);

        // Receive things
        char data[4096] = {};
        struct sockaddr_storage dev_null;
        socklen_t l = saddr_size;
        auto start = std::chrono::steady_clock::now();
        auto data_received = recvfrom(s, data, sizeof(data) - 2, 0, reinterpret_cast<struct sockaddr *>(&dev_null), &l);
        auto end = std::chrono::steady_clock::now();
        if(data_received != SOCKET_ERROR) {
            finished_packet.ping = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::pair<std::string, std::string> kv;
            bool key = true;
            char *str_start = data + 1;
            for(char *c = str_start; *c; c++) {
                bool break_now = false;
                if(*c != '\\' && c[1] == 0) {
                    c++;
                    break_now = true;
                }
                if(*c == '\\') {
                    if(key) {
                        kv.first = std::string(str_start, c - str_start);
                    }
                    else {
                        // Strip invalid/whitespace characters from the start. If the string is all invalid, don't strip the last character.
                        for(char *k = str_start; k + 1 < c && *k <= 0x20; k++, str_start++);

                        // And here we go!
                        kv.second = std::string(str_start, c - str_start);

                        // Next, replace any unknown characters with '?'
                        for(char &c : kv.second) {
                            if(c < 0x20) {
                                c = '?';
                            }
                        }

                        finished_packet.query_data.insert_or_assign(kv.first, kv.second);
                        kv = {};
                    }
                    key = !key;
                    str_start = c + 1;
                }
                if(break_now) {
                    break;
                }
            }

            // = std::vector<char>(data, data + data_received);
            finished_packet.error = QueryPacketDone::Error::NONE;
        }
        else {
            finished_packet.error = QueryPacketDone::Error::TIMED_OUT;
        }
        closesocket(s);

        return finished_packet;
    }

    static void query_list(const std::vector<Bookmark> &bookmarks) {
        finished_packets.clear();

        for(auto &b : bookmarks) {
            finished_packets.push_back(query_server(b));
        }

        querying.unlock();
    }

    static void show_list() {
        if(!querying.try_lock()) {
            return;
        }
        querying.unlock();

        // Show the results
        auto &resolution = get_resolution();
        bool can_use_tabs = static_cast<float>(resolution.width) / resolution.height > (4.05F / 3.0F);
        if(can_use_tabs) {
            console_output(localize("chimera_bookmark_list_header"));
        }
        std::size_t q = 0;
        std::size_t success = 0;

        for(auto &p : finished_packets) {
            q++;
            switch(p.error) {
                case QueryPacketDone::Error::NONE: {
                    float red = 0.0F;
                    float green = 0.0F;

                    // <15 ms = green
                    if(p.ping < 15) {
                        green = 1.0F;
                    }

                    // 30 - 80 ms = green to yellow
                    else if(p.ping < 80) {
                        green = 1.0F;
                        red = (p.ping - 15) / static_cast<float>(80 - 15);
                    }

                    // 80 - 200 ms = yellow to red
                    else if(p.ping < 200) {
                        green = 1.0F - (p.ping - 80) / static_cast<float>(200 - 80);
                        red = 1.0F;
                    }

                    // 200 ms - 300 ms = darker red
                    else if(p.ping < 300) {
                        red = 1.0F - (p.ping - 200) / static_cast<float>(300 - 200) * 0.7F;
                    }

                    // 300+ ping = dark red
                    else {
                        red = 0.7F;
                    }

                    success++;

                    // Sometimes the game variant isn't set to anything (default gametype I think?). In this case, use the gametype.
                    const char *gametype = p.get_data_for_key("gamevariant");
                    if(!gametype[0]) {
                        gametype = p.get_data_for_key("gametype");
                    }

                    if(can_use_tabs) {
                        console_output(ConsoleColor { 1.0F, red, green, 0.25F }, "%zu. %s|t%s|t%s|t%s / %s|r%zu ms", q, p.get_data_for_key("hostname"), p.get_data_for_key("mapname"), gametype, p.get_data_for_key("numplayers"), p.get_data_for_key("maxplayers"), p.ping);
                    }
                    else {
                        console_output(ConsoleColor { 1.0F, red, green, 0.25F }, "%zu. %s (%s; %s / %s) - %zu ms", q, p.get_data_for_key("hostname"), p.get_data_for_key("mapname"), p.get_data_for_key("numplayers"), p.get_data_for_key("maxplayers"), p.ping);
                    }

                    break;
                }
                case QueryPacketDone::Error::FAILED_TO_RESOLVE:
                    console_output(ConsoleColor { 1.0F, 1.0F, 0.5F, 0.5F }, "%zu. %s%s%s:%zu|t%s", q, p.b.brackets ? "[" : "", p.b.address, p.b.brackets ? "]" : "", p.b.port, localize("chimera_bookmark_list_error_failed_to_resolve"));
                    break;
                case QueryPacketDone::Error::TIMED_OUT:
                    console_output(ConsoleColor { 1.0F, 1.0F, 0.5F, 0.5F }, "%zu. %s%s%s:%zu|t%s", q, p.b.brackets ? "[" : "", p.b.address, p.b.brackets ? "]" : "", p.b.port, localize("chimera_bookmark_list_error_timed_out"));
                    break;
            }
        }

        remove_preframe_event(show_list);
    }

    bool history_list_command(int, const char **) {
        if(!querying.try_lock()) {
            console_error(localize("chimera_bookmark_list_command_busy"));
            return false;
        }
        console_output(localize("chimera_history_list_command_querying"));
        add_preframe_event(show_list);
        std::thread(query_list, load_bookmarks_file("history.txt")).detach();
        return true;
    }

    bool bookmark_list_command(int, const char **) {
        if(!querying.try_lock()) {
            console_error(localize("chimera_bookmark_list_command_busy"));
            return false;
        }
        console_output(localize("chimera_bookmark_list_command_querying"));
        add_preframe_event(show_list);
        std::thread(query_list, load_bookmarks_file("bookmark.txt")).detach();
        return true;
    }

    bool bookmark_add_command(int argc, const char **argv) {
        auto bookmarks = load_bookmarks_file("bookmark.txt");
        Bookmark new_bookmark = {};
        if(argc == 0) {
            auto history = load_bookmarks_file("history.txt");
            if(history.size() == 0) {
                console_error(localize("chimera_bookmark_add_no_recent_servers"));
                return false;
            }
            new_bookmark = history[0];
        }
        else {
            auto potential_bookmark = parse_bookmark(*argv);
            if(!potential_bookmark.has_value()) {
                console_error(localize("chimera_bookmark_error_invalid"));
                return false;
            }
            new_bookmark = *potential_bookmark;

            // If we have a password, add that too
            if(argc == 2) {
                if(std::strlen(argv[1]) < sizeof(new_bookmark.password)) {
                    std::strncpy(new_bookmark.password, argv[1], sizeof(new_bookmark.password) - 1);
                }
                else {
                    console_error(localize("chimera_bookmark_error_password_too_long"));
                    return false;
                }
            }
        }

        // Simply replace if found
        for(std::size_t b = 0; b < bookmarks.size(); b++) {
            auto &bookmark = bookmarks[b];
            if(std::strcmp(bookmark.address, new_bookmark.address) == 0 && bookmark.port == new_bookmark.port) {
                bookmark = new_bookmark;
                console_output(localize("chimera_bookmark_add_success"), bookmark.brackets ? "[" : "", bookmark.address, bookmark.brackets ? "]" : "", bookmark.port, b);
                return true;
            }
        }

        bookmarks.emplace_back(new_bookmark);
        console_output(localize("chimera_bookmark_add_success"), new_bookmark.brackets ? "[" : "", new_bookmark.address, new_bookmark.brackets ? "]" : "", new_bookmark.port, bookmarks.size());
        save_bookmarks_file("bookmark.txt", bookmarks);

        return true;
    }

    bool bookmark_delete_command(int argc, const char **argv) {
        auto bookmarks = load_bookmarks_file("bookmark.txt");
        Bookmark delete_bookmark = {};
        if(argc == 0) {
            auto history = load_bookmarks_file("history.txt");
            if(history.size() == 0) {
                console_error(localize("chimera_bookmark_add_no_recent_servers"));
                return false;
            }
            delete_bookmark = history[0];
        }
        else {
            try {
                std::size_t index = static_cast<std::size_t>(std::stoul(*argv));
                if(index > bookmarks.size() || index < 1) {
                    console_error(localize("chimera_bookmark_error_not_found"));
                    return false;
                }
                delete_bookmark = bookmarks[index - 1];
            }
            catch(std::exception &) {
                auto potential_bookmark = parse_bookmark(*argv);
                if(!potential_bookmark.has_value()) {
                    console_error(localize("chimera_bookmark_error_invalid"));
                    return false;
                }
                delete_bookmark = *potential_bookmark;
            }
        }

        // Remove
        bool success = false;

        // Delete if found
        for(std::size_t b = 0; b < bookmarks.size(); b++) {
            auto &bookmark = bookmarks[b];
            if(std::strcmp(bookmark.address, delete_bookmark.address) == 0 && bookmark.port == delete_bookmark.port) {
                success = true;
                console_output(localize("chimera_bookmark_delete_success"), bookmark.brackets ? "[" : "", bookmark.address, bookmark.brackets ? "]" : "", bookmark.port);
                bookmarks.erase(bookmarks.begin() + b);
                b--;
            }
        }
        if(success) {
            save_bookmarks_file("bookmark.txt", bookmarks);
        }
        else {
            console_error(localize("chimera_bookmark_error_not_found"));
        }

        return success;
    }

    static void join_bookmark(const Bookmark &bookmark) {
        char connect_command[256];
        std::snprintf(connect_command, sizeof(connect_command), "connect %s%s%s:%u \"%s\"", bookmark.brackets ? "[" : "", bookmark.address, bookmark.brackets ? "]" : "", bookmark.port, bookmark.password);
        execute_script(connect_command);
    }

    bool bookmark_connect_command(int, const char **argv) {
        auto bookmarks = load_bookmarks_file("bookmark.txt");
        std::size_t index;
        try {
            index = static_cast<std::size_t>(std::stoul(*argv));
        }
        catch(std::exception &) {
            console_error(localize("chimera_bookmark_error_invalid"));
            return false;
        }
        if(index < 1 || index > bookmarks.size()) {
            console_error(localize("chimera_bookmark_error_invalid"));
            return false;
        }
        join_bookmark(bookmarks[index - 1]);
        return true;
    }

    bool history_connect_command(int, const char **argv) {
        auto history = load_bookmarks_file("history.txt");
        std::size_t index;
        try {
            index = static_cast<std::size_t>(std::stoul(*argv));
        }
        catch(std::exception &) {
            console_error(localize("chimera_bookmark_error_invalid"));
            return false;
        }
        if(index < 1 || index > history.size()) {
            console_error(localize("chimera_bookmark_error_invalid"));
            return false;
        }
        join_bookmark(history[index - 1]);
        return true;
    }
}
