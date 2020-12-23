// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <cstring>
#include <thread>

#define CURL_STATICLIB
#include <curl/curl.h>
#include <filesystem>

#include "hac_map_downloader.hpp"

void HACMapDownloader::dispatch_thread_function(HACMapDownloader *downloader) {
    // Set the URL
    CURLcode result;

    // Determine the first repo to use
    downloader->mutex.lock();
    auto preferred_server_hold = downloader->preferred_server_node;
    downloader->mutex.unlock();
    bool preferred_failed = !preferred_server_hold.has_value();
    unsigned int repo = preferred_server_hold.value_or(1);

    std::string map_formatted;
    for(char &c : downloader->map) {
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            map_formatted += c;
        }
        else {
            char code[4];
            std::snprintf(code, sizeof(code), "%%%02X", c);
            map_formatted += code;
        }
    }
    do {
        char url[255];
        std::snprintf(url, sizeof(url), "http://maps%u.halonet.net/halonet/locator.php?format=inv&map=%s&type=%s", repo, map_formatted.data(), downloader->game_engine.data());
        
        // Close if needed
        if(downloader->output_file_handle) {
            std::fclose(downloader->output_file_handle);
        }
        
        // Let's begin
        downloader->output_file_handle = std::fopen(downloader->output_file.string().c_str(), "wb");

        // If we failed to open, give up and close
        if(downloader->output_file_handle == nullptr) {
            downloader->status = HACMapDownloader::DOWNLOAD_STAGE_FAILED;
            return;
        }
        
        // Otherwise, let's begin
        curl_easy_setopt(downloader->curl, CURLOPT_URL, url);
        downloader->download_started = Clock::now();

        // Did we fail?
        if((result = curl_easy_perform(downloader->curl)) != CURLE_OK) {
            // Did our preferred one fail? If so, give up
            if(preferred_failed && result != CURLcode::CURLE_COULDNT_RESOLVE_HOST) {
                break;
            }

            if(preferred_server_hold.has_value()) {
                if(preferred_failed) {
                    repo++;
                }
                else {
                    repo = 1;
                    preferred_failed = true;
                }
            }
            else {
                repo++;
            }
        }

        // Cancel?
        downloader->mutex.lock();
        if(downloader->status == DownloadStage::DOWNLOAD_STAGE_CANCELING) {
            curl_easy_cleanup(downloader->curl);
            downloader->curl = nullptr;
            downloader->mutex.unlock();
            return;
        }
        downloader->mutex.unlock();
    }
    while(result != CURLcode::CURLE_OK);

    // Note that we're extracting; clean up CURL
    downloader->mutex.lock();
    curl_easy_cleanup(downloader->curl);
    downloader->curl = nullptr;

    // If we were successful, do it
    if(result == CURLcode::CURLE_OK) {
        // Write the last data
        std::fwrite(downloader->buffer.data(), downloader->buffer_used, 1, downloader->output_file_handle);
        downloader->buffer_used = 0;
        downloader->buffer.clear();

        // Close the file handle
        if(downloader->output_file_handle) {
            std::fclose(downloader->output_file_handle);
        }
        downloader->output_file_handle = nullptr;

        downloader->status = DOWNLOAD_STAGE_COMPLETE;
    }
    else {
        downloader->status = DOWNLOAD_STAGE_FAILED;
    }

    downloader->mutex.unlock();
}

void HACMapDownloader::set_preferred_server_node(const std::optional<unsigned int> &server) noexcept {
    this->mutex.lock();
    this->preferred_server_node = server;
    this->mutex.unlock();
}

std::size_t HACMapDownloader::get_download_speed() noexcept {
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
class HACMapDownloader::HACMapDownloaderCallback {
public:
    // When we've received data, put it in here
    static size_t write_callback(const std::byte *ptr, std::size_t, std::size_t nmemb, HACMapDownloader *userdata) {
        userdata->mutex.lock();

        // If we're canceling, stop
        if(userdata->status == HACMapDownloader::DOWNLOAD_STAGE_CANCELING) {
            userdata->mutex.unlock();
            return 0;
        }

        userdata->status = HACMapDownloader::DOWNLOAD_STAGE_DOWNLOADING;
        if(userdata->buffer_used + nmemb > userdata->buffer.size()) {
            std::fwrite(userdata->buffer.data(), userdata->buffer_used, 1, userdata->output_file_handle);
            std::fwrite(ptr, nmemb, 1, userdata->output_file_handle);
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
    static int progress_callback(HACMapDownloader *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) {
        clientp->mutex.lock();
        clientp->downloaded_size = dlnow;
        clientp->total_size = dltotal;
        clientp->mutex.unlock();
        return 0;
    }
};

const std::string &HACMapDownloader::get_map() const noexcept {
    return this->map;
}

void HACMapDownloader::cancel() noexcept {
    this->mutex.lock();
    if(this->status == DOWNLOAD_STAGE_CANCELED || this->status == DOWNLOAD_STAGE_FAILED) {
        if(std::filesystem::exists(this->output_file)) {
            if(this->output_file_handle) {
                std::fclose(this->output_file_handle);
            }
            std::filesystem::remove(this->output_file);
        }
    }
    if(this->status == DOWNLOAD_STAGE_CANCELED) {
        this->mutex.unlock();
        return;
    }
    bool not_finished = false;
    if(!this->is_finished_no_mutex()) {
        this->status = HACMapDownloader::DOWNLOAD_STAGE_CANCELING;
        not_finished = true;
    }
    this->mutex.unlock();

    // If we aren't finished, then set the status to canceled and delete things
    this->dispatch_thread.join();
    if(not_finished) {
        this->mutex.lock();
        this->status = DOWNLOAD_STAGE_CANCELED;
        if(this->output_file_handle) {
            std::fclose(this->output_file_handle);
            this->output_file_handle = nullptr;
        }
        this->mutex.unlock();
    }
}

// Set up stuff
void HACMapDownloader::dispatch() {
    // Lock the mutex
    this->mutex.lock();
    if(this->curl) {
        this->mutex.unlock();
        throw std::exception();
        return;
    }

    // Initialize cURL as well as the downloader variables
    this->curl = curl_easy_init();

    // Set our callbacks
    curl_easy_setopt(this->curl, CURLOPT_XFERINFOFUNCTION, HACMapDownloaderCallback::progress_callback);
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, HACMapDownloaderCallback::write_callback);

    // Set the data passed to the callbacks so our class can be updated
    curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(this->curl, CURLOPT_PROGRESSDATA, this);

    // Enable progress bar
    curl_easy_setopt(this->curl, CURLOPT_NOPROGRESS, 0);

    // Fail on error
    curl_easy_setopt(this->curl, CURLOPT_FAILONERROR, 1);

    // 10 second timeout
    curl_easy_setopt(this->curl, CURLOPT_CONNECTTIMEOUT, 10L);

    // Set the download stage to starting
    this->status = HACMapDownloader::DOWNLOAD_STAGE_STARTING;

    // Hold 1 MiB of data in memory
    this->buffer.insert(this->buffer.end(), 1024 * 1024, std::byte());
    this->buffer_used = 0;

    // Set the number of bytes downloaded to 0
    this->downloaded_size = 0;
    this->total_size = 0;

    // Make the thread happen
    this->dispatch_thread = std::thread(HACMapDownloader::dispatch_thread_function, this);

    // Unlock
    this->mutex.unlock();
}

HACMapDownloader::DownloadStage HACMapDownloader::get_status() noexcept {
    this->mutex.lock();
    auto return_value = this->status;
    this->mutex.unlock();
    return return_value;
}

std::size_t HACMapDownloader::get_downloaded_size() noexcept {
    this->mutex.lock();
    std::size_t return_value = this->downloaded_size;
    this->mutex.unlock();
    return return_value;
}

std::size_t HACMapDownloader::get_total_size() noexcept {
    this->mutex.lock();
    std::size_t return_value = this->total_size;
    this->mutex.unlock();
    return return_value;
}

bool HACMapDownloader::is_finished_no_mutex() const noexcept {
    return this->status == DOWNLOAD_STAGE_COMPLETE || this->status == DOWNLOAD_STAGE_FAILED || this->status == DOWNLOAD_STAGE_NOT_STARTED || this->status == DOWNLOAD_STAGE_CANCELED;
}

bool HACMapDownloader::is_finished() noexcept {
    auto m = this->mutex.try_lock();
    if(!m) {
        return false;
    }
    bool finished = this->is_finished_no_mutex();
    this->mutex.unlock();
    return finished;
}

HACMapDownloader::HACMapDownloader(const char *map, const char *output_file, const char *game_engine) : map(map), output_file(output_file), game_engine(game_engine) {
    for(char &c : this->map) {
        c = std::tolower(c);
    }
}
HACMapDownloader::~HACMapDownloader() {
    this->cancel();
}
