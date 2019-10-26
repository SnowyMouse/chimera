#include <cstdio>
#include <cstring>
#include <thread>
#include <curl/curl.h>
#include "hac_map_downloader.hpp"

void HACMapDownloader::dispatch_thread(HACMapDownloader *downloader) {
    // Set the URL
    CURLcode result;
    unsigned int repo = 1;
    do {
        char url[255];
        std::snprintf(url, sizeof(url), "http://maps%u.halonet.net/halonet/locator.php?map=%s", repo, downloader->map.data());
        curl_easy_setopt(downloader->curl, CURLOPT_URL, url);
        result = curl_easy_perform(downloader->curl);
        repo++;
    }
    while(result != CURLcode::CURLE_COULDNT_RESOLVE_HOST && result != CURLcode::CURLE_OK);

    downloader->mutex.lock();
    curl_easy_cleanup(downloader->curl);
    downloader->curl = nullptr;
    downloader->finished = true;
    downloader->successful = (result == CURLcode::CURLE_OK);
    downloader->mutex.unlock();
}

// Callback class
class HACMapDownloader::HACMapDownloaderCallback {
public:
    // When we've received data, put it in here
    static size_t write_callback(std::byte *ptr, std::size_t size, std::size_t nmemb, HACMapDownloader *userdata) {
        userdata->mutex.lock();
        userdata->compressed_data.insert(userdata->compressed_data.end(), ptr, ptr + nmemb);
        userdata->mutex.unlock();
        return nmemb;
    }

    // When progress has been made, record it here
    static int progress_callback(HACMapDownloader *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        clientp->mutex.lock();
        clientp->downloaded_size = dlnow;
        clientp->total_size = dltotal;
        clientp->mutex.unlock();
        return 0;
    }
};

// Set up stuff
void HACMapDownloader::dispatch() {
    if(this->curl) {
        std::terminate();
        return;
    }

    this->temp_file_handle = std::fopen(this->temp_file.data(), "wb");
    if(!this->temp_file_handle) {
        this->successful = false;
        this->finished = true;
        return;
    }

    // Initialize cURL as well as the downloader variables
    this->curl = curl_easy_init();
    this->finished = false;
    this->successful = false;
    this->compressed_data.clear();

    // Set our callbacks
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, HACMapDownloaderCallback::progress_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HACMapDownloaderCallback::write_callback);

    // Set the data passed to the callbacks so our class can be updated
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);

    // Enable progress bar
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

    // Fail on error
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

    // 10 second timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    std::thread(HACMapDownloader::dispatch_thread, this).detach();
}

bool HACMapDownloader::is_finished() noexcept {
    bool return_value = false;
    if(this->mutex.try_lock()) {
        return_value = this->finished;
        this->mutex.unlock();
    }
    return return_value;
}

bool HACMapDownloader::is_successful() noexcept {
    this->mutex.lock();
    std::size_t return_value = this->successful;
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

const std::vector<std::byte> *HACMapDownloader::get_map_data() noexcept {
    if(this->is_successful()) {
        return &this->decompressed_data;
    }
    else {
        return nullptr;
    }
}

HACMapDownloader::HACMapDownloader(const char *map, const char *temp_file, const char *map_file) : map(map), temp_file(temp_file), map_file(map_file) {}
HACMapDownloader::~HACMapDownloader() {
    while(!this->finished);
}
