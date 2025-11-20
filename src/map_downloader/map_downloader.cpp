// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <cstring>
#include <thread>

#define CURL_STATICLIB
#include <curl/curl.h>
#include <filesystem>
#include <regex>

#include "map_downloader.hpp"

/**
 * Split a string on a delimiter
 * @param str   the string to split
 * @param delim the delimiter to split on
 * @return A vector of strings
 */
std::vector<std::string> split(std::string str, std::string delim) {
    size_t start = 0, end = 0, d_len = delim.length();
    std::vector<std::string> ret;

    while ((end = str.find(delim, start)) != std::string::npos) {
        ret.push_back(str.substr(start, end - start));
        start = end + d_len;
    }

    ret.push_back(str.substr(start));
    return ret;
}


void MapDownloader::dispatch_thread_function(MapDownloader *downloader) {
    CURLcode result = CURLcode::CURLE_FAILED_INIT;

    // Format everything except the mirror into the url template
    char* map_urlencoded = curl_easy_escape(downloader->curl, downloader->map.c_str(), 0);
    char* password_urlencoded = curl_easy_escape(downloader->curl, downloader->password.c_str(), 0);

    std::string partial_url;
    partial_url = std::regex_replace(downloader->url_template, std::regex("\\{map\\}"), map_urlencoded);
    partial_url = std::regex_replace(partial_url, std::regex("\\{game\\}"), downloader->game_engine);
    partial_url = std::regex_replace(partial_url, std::regex("\\{server\\}"), downloader->server);
    partial_url = std::regex_replace(partial_url, std::regex("\\{password\\}"), password_urlencoded);

    curl_free(map_urlencoded);
    curl_free(password_urlencoded);

    // Grab the list of comma-separated mirrors from the template
    static const std::regex mirror_regex("\\{mirror<([^>]+)>\\}");
    std::smatch match;
    std::string mirror_str = "";
    if (std::regex_search(downloader->url_template, match, mirror_regex)){
        mirror_str = match[1];
    }
    std::vector<std::string> mirrors = split(mirror_str, ",");

    // Try each mirror
    for (auto mirror : mirrors){
        // Finish formatting the download URL
        std::string url = std::regex_replace(partial_url, mirror_regex, mirror);

        // Close if needed
        if(downloader->output_file_handle) {
            std::fclose(downloader->output_file_handle);
        }

        // Let's begin
        downloader->output_file_handle = std::fopen(downloader->output_file.string().c_str(), "wb");

        // If we failed to open, give up
        if(downloader->output_file_handle == nullptr) {
            downloader->mutex.lock();
            downloader->status = DOWNLOAD_STAGE_FAILED;
            downloader->mutex.unlock();
            break;
        }

        // Otherwise, let's begin
        curl_easy_setopt(downloader->curl, CURLOPT_URL, url.data());
        downloader->download_started = Clock::now();
        result = curl_easy_perform(downloader->curl);

        // Cancel?
        downloader->mutex.lock();
        if(downloader->status == DownloadStage::DOWNLOAD_STAGE_CANCELING) {
            downloader->mutex.unlock();
            break;
        }
        downloader->mutex.unlock();

        if (result == CURLcode::CURLE_OK){
            break;
        }
    }

    // Note that we're extracting; clean up CURL
    downloader->mutex.lock();
    curl_easy_cleanup(downloader->curl);
    downloader->curl = nullptr;

    // If we were successful, do it
    if(result == CURLcode::CURLE_OK) {
        // Write the last data
        std::fwrite(downloader->buffer.data(), downloader->buffer_used, 1, downloader->output_file_handle);
        downloader->written_size += downloader->buffer_used;
        downloader->buffer_used = 0;
        downloader->buffer.clear();

        // Close the file handle
        if(downloader->output_file_handle) {
            std::fclose(downloader->output_file_handle);
        }
        downloader->output_file_handle = nullptr;

        // If we downloaded enough that we definitely checked it, yay!
        downloader->status = downloader->written_size >= 0x800 ? DownloadStage::DOWNLOAD_STAGE_COMPLETE : DownloadStage::DOWNLOAD_STAGE_FAILED;
    }
    else {
        // Failed/canceled - clean up the output file
        if(downloader->output_file_handle) {
            std::fclose(downloader->output_file_handle);
        }
        downloader->output_file_handle = nullptr;
        if(std::filesystem::exists(downloader->output_file)) {
            std::filesystem::remove(downloader->output_file);
        }

        downloader->status = DownloadStage::DOWNLOAD_STAGE_FAILED;
    }

    downloader->mutex.unlock();
}

void MapDownloader::set_server_info(const std::string &server, const std::string &password) noexcept {
    this->mutex.lock();
    this->server = server;
    this->password = password;
    this->mutex.unlock();
}

std::size_t MapDownloader::get_download_speed() noexcept {
    // If we haven't started, return 0
    if(this->downloaded_size == 0) {
        return 0.0;
    }

    auto now = Clock::now();
    auto difference = now - this->download_started;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(difference).count();

    // Don't divide by zero
    if(ms <= 0) {
        return 0.0;
    }

    return (this->downloaded_size) / ms;
}

// Callback class
class MapDownloader::MapDownloaderCallback {
public:
    // When we've received data, put it in here
    static size_t write_callback(const std::byte *ptr, std::size_t, std::size_t nmemb, MapDownloader *userdata) {
        userdata->mutex.lock();

        // If we're canceling, stop
        if(userdata->status == MapDownloader::DOWNLOAD_STAGE_CANCELING) {
            userdata->mutex.unlock();
            return 0;
        }

        // Check if this is a bad download
        std::byte header_data[0x800];
        if(userdata->written_size == 0 && userdata->buffer_used < sizeof(header_data) && nmemb + userdata->buffer_used >= sizeof(header_data)) {
            std::memcpy(header_data, userdata->buffer.data(), userdata->buffer_used);
            std::memcpy(header_data + userdata->buffer_used, ptr, sizeof(header_data) - userdata->buffer_used);
            bool bad_header = true;
            if(*reinterpret_cast<std::uint32_t *>(header_data) == 0x68656164 && *reinterpret_cast<std::uint32_t *>(header_data + 0x7FC) == 0x666F6F74) {
                bad_header = false;
            }
            else if(*reinterpret_cast<std::uint32_t *>(header_data + 0x2C0) == 0x45686564 && *reinterpret_cast<std::uint32_t *>(header_data + 0x5F0) == 0x47666F74) {
                bad_header = false;
            }
            if(bad_header) {
                userdata->status = MapDownloader::DOWNLOAD_STAGE_FAILED;
                userdata->mutex.unlock();
                return 0;
            }

        }

        userdata->status = MapDownloader::DOWNLOAD_STAGE_DOWNLOADING;

        if(userdata->buffer_used + nmemb > userdata->buffer.size()) {
            std::fwrite(userdata->buffer.data(), userdata->buffer_used, 1, userdata->output_file_handle);
            std::fwrite(ptr, nmemb, 1, userdata->output_file_handle);
            userdata->written_size += userdata->buffer_used + nmemb;
            userdata->buffer_used = 0;
        }
        else {
            std::copy(ptr, ptr + nmemb, userdata->buffer.data() + userdata->buffer_used);
            userdata->buffer_used += nmemb;
        }

        userdata->mutex.unlock();
        return nmemb;
    }

    // When progress has been made, record it here
    static int progress_callback(MapDownloader *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) {
        clientp->mutex.lock();
        clientp->downloaded_size = dlnow;
        clientp->total_size = dltotal;
        clientp->mutex.unlock();
        return 0;
    }
};

const std::string &MapDownloader::get_map() const noexcept {
    return this->map;
}

void MapDownloader::cancel() noexcept {
    this->mutex.lock();
    if(this->status == DOWNLOAD_STAGE_CANCELED) {
        // Already cancelled
        this->mutex.unlock();
        return;
    }

    // Set the status to canceling and wait for the thread to join
    if(!this->is_finished_no_mutex()) {
        this->status = DownloadStage::DOWNLOAD_STAGE_CANCELING;
    }
    this->mutex.unlock();

    this->dispatch_thread.join();

    this->mutex.lock();
    this->status = DownloadStage::DOWNLOAD_STAGE_CANCELED;
    this->mutex.unlock();
}

// Set up stuff
void MapDownloader::download(const char *map, const char *output_file, const char *game_engine) {
    // Lock the mutex
    this->mutex.lock();
    if(this->curl) {
        this->mutex.unlock();
        throw std::exception();
        return;
    }

    // Set download variables
    this->map = map;
    this->output_file = output_file;
    this->game_engine = game_engine;
    for(char &c : this->map) {
        c = std::tolower(c, std::locale("C"));
    }

    // Initialize cURL as well as the downloader variables
    this->curl = curl_easy_init();

    // Set our callbacks
    curl_easy_setopt(this->curl, CURLOPT_XFERINFOFUNCTION, MapDownloaderCallback::progress_callback);
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, MapDownloaderCallback::write_callback);

    // Set the data passed to the callbacks so our class can be updated
    curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(this->curl, CURLOPT_PROGRESSDATA, this);

    // Enable progress bar
    curl_easy_setopt(this->curl, CURLOPT_NOPROGRESS, 0);

    // Fail on error
    curl_easy_setopt(this->curl, CURLOPT_FAILONERROR, 1);

    // Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // 10 second timeout
    curl_easy_setopt(this->curl, CURLOPT_CONNECTTIMEOUT, 10L);

    curl_easy_setopt(this->curl, CURLOPT_USERAGENT, "Chimera MapDownloader/1.0");

    // Set the download stage to starting
    this->status = MapDownloader::DOWNLOAD_STAGE_STARTING;

    // Hold 1 MiB of data in memory
    this->buffer.insert(this->buffer.end(), 1024 * 1024, std::byte());
    this->buffer_used = 0;

    // Set the number of bytes downloaded to 0
    this->downloaded_size = 0;
    this->written_size = 0;
    this->total_size = 0;

    // Make the thread happen
    this->dispatch_thread = std::thread(MapDownloader::dispatch_thread_function, this);

    // Unlock
    this->mutex.unlock();
}

MapDownloader::DownloadStage MapDownloader::get_status() noexcept {
    this->mutex.lock();
    auto return_value = this->status;
    this->mutex.unlock();
    return return_value;
}

std::size_t MapDownloader::get_downloaded_size() noexcept {
    this->mutex.lock();
    std::size_t return_value = this->downloaded_size;
    this->mutex.unlock();
    return return_value;
}

std::size_t MapDownloader::get_total_size() noexcept {
    this->mutex.lock();
    std::size_t return_value = this->total_size;
    this->mutex.unlock();
    return return_value;
}

bool MapDownloader::is_finished_no_mutex() const noexcept {
    return this->status == DOWNLOAD_STAGE_COMPLETE || this->status == DOWNLOAD_STAGE_FAILED || this->status == DOWNLOAD_STAGE_NOT_STARTED || this->status == DOWNLOAD_STAGE_CANCELED;
}

bool MapDownloader::is_finished() noexcept {
    auto m = this->mutex.try_lock();
    if(!m) {
        return false;
    }
    bool finished = this->is_finished_no_mutex();
    this->mutex.unlock();
    return finished;
}

MapDownloader::MapDownloader(const std::string &url_template) : url_template(url_template) {
    this->server = "";
    this->password = "";
    this->map = "";
}
MapDownloader::~MapDownloader() {
    this->cancel();
}
