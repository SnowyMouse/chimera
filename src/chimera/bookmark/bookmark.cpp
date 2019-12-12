#include <winsock2.h>
#include <ws2tcpip.h>
#include "bookmark.hpp"
#include "../chimera.hpp"
#include "../event/frame.hpp"
#include <windows.h>
#include <cstring>
#include <fstream>
#include "../event/connect.hpp"
#include "../output/output.hpp"
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
            Bookmark b = {};

            // Get the address
            std::size_t i;
            std::string address;
            std::size_t address_length = 0;
            for(i = 0; i < sizeof(b.address) - 1 && i < line.size() && line[i] != '\r' && line[i] != '\n' && line[i] != ':'; i++, address_length++);
            if(address_length < 2) {
                continue;
            }
            else if(line[0] == '[' && line[address_length - 1] == ']') {
                address = line.substr(1, address_length - 1);
                b.brackets = true;
            }
            else {
                address = line.substr(0, address_length);
            }
            std::strncpy(b.address, address.data(), sizeof(b.address) - 1);

            // Now the port
            if(line[i++] == ':') {
                std::size_t port_length = 0;
                for(i = 0; i < line.size() && line[i] != '\r' && line[i] != '\n'; i++, port_length++);
                try {
                    b.port = static_cast<std::uint16_t>(std::stoi(line.substr(address_length + 1, port_length)));
                }
                catch(std::exception &) {
                    continue;
                }
            }
            else {
                continue;
            }

            // Lastly, the password (if present)
            if(line[i++] == ' ') {
                std::size_t s;
                for(s = 0; s < sizeof(b.password) - 1 && i < line.size() && line[i] != '\r' && line[i] != '\n'; i++, s++) {
                    b.password[s] = line[i];
                }
                b.password[s] = 0;
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
            if(b.password[0]) {
                std::snprintf(line, sizeof(line), "%s%s%s:%u %s", b.brackets ? "[" : "", b.address, b.brackets ? "]" : "", b.port, b.password);
            }
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

    static std::mutex querying;
    struct QueryPacketDone {
        enum Error {
            NONE,
            FAILED_TO_RESOLVE,
            TIMED_OUT
        };

        Bookmark b;
        std::vector<std::pair<std::string, std::string>> query_data;
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
    static std::vector<QueryPacketDone> finished_packets;

    static void query_list(const std::vector<Bookmark> &bookmarks) {
        finished_packets.clear();

        for(auto &b : bookmarks) {
            auto &finished_packet = finished_packets.emplace_back();
            finished_packet.b = b;

            struct addrinfo *address;

            // Lookup it
            char port[6] = {};
            std::snprintf(port, sizeof(port), "%u", b.port);
            int q = getaddrinfo(b.address, "2302", nullptr, &address);
            if(q != 0) {
                finished_packet.error = QueryPacketDone::Error::FAILED_TO_RESOLVE;
                continue;
            }
            auto family = address->ai_family;
            if(family != AF_INET && family != AF_INET6) {
                freeaddrinfo(address);
                finished_packet.error = QueryPacketDone::Error::FAILED_TO_RESOLVE;
                continue;
            }

            // Do the thing
            struct sockaddr_storage saddr = {};
            std::size_t saddr_size = address->ai_addrlen;

            // Free it all
            std::memcpy(&saddr, address->ai_addr, saddr_size);
            freeaddrinfo(address);

            // Do socket things
            SOCKET s = socket(family, SOCK_DGRAM, IPPROTO_UDP);
            DWORD opt = 1000;
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
                            kv.second = std::string(str_start, c - str_start);
                            for(char &c : kv.second) {
                                if(c < 0x20) {
                                    c = '?';
                                }
                            }
                            finished_packet.query_data.push_back(std::move(kv));
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

                    if(p.ping < 20) {
                        green = 1.0F;
                    }
                    else if(p.ping < 120) {
                        green = 1.0F - (p.ping - 20) / 100.0F;
                        red = 0.2F + (p.ping - 20) / 100.0F * 0.8F;
                    }
                    else {
                        red = 1.0F;
                    }

                    success++;

                    if(can_use_tabs) {
                        console_output(ConsoleColor { 1.0F, red, green, 0.25F }, "%zu. %s|t%s|t%s|t%s / %s|r%zu ms", q, p.get_data_for_key("hostname"), p.get_data_for_key("mapname"), p.get_data_for_key("gamevariant"), p.get_data_for_key("numplayers"), p.get_data_for_key("maxplayers"), p.ping);
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

    bool history_list_command(int argc, const char **argv) {
        if(!querying.try_lock()) {
            console_error(localize("chimera_bookmark_list_command_busy"));
            return false;
        }
        console_output(localize("chimera_history_list_command_querying"));
        add_preframe_event(show_list);
        std::thread(query_list, load_bookmarks_file("history.txt")).detach();
        return true;
    }

    bool bookmark_list_command(int argc, const char **argv) {
        if(!querying.try_lock()) {
            console_error(localize("chimera_bookmark_list_command_busy"));
            return false;
        }
        console_output(localize("chimera_bookmark_list_command_querying"));
        add_preframe_event(show_list);
        std::thread(query_list, load_bookmarks_file("bookmark.txt")).detach();
        return true;
    }
}
