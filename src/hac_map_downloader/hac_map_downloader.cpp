#include <cstdio>
#include <cstring>
#include <thread>

#define CURL_STATICLIB
#include <curl/curl.h>

#define TEST_EXTRACTION_ONLY

#include "hac_map_downloader.hpp"

void HACMapDownloader::dispatch_thread(HACMapDownloader *downloader) {
    #ifndef TEST_EXTRACTION_ONLY
    // Set the URL
    CURLcode result;
    unsigned int repo = 1;
    do {
        char url[255];
        std::snprintf(url, sizeof(url), "http://maps%u.halonet.net/halonet/locator.php?format=7z&map=%s", repo, downloader->map.data());
        curl_easy_setopt(downloader->curl, CURLOPT_URL, url);
        result = curl_easy_perform(downloader->curl);
        repo++;
    }
    while(result != CURLcode::CURLE_COULDNT_RESOLVE_HOST && result != CURLcode::CURLE_OK);

    // Note that we're extracting; clean up CURL
    downloader->mutex.lock();
    curl_easy_cleanup(downloader->curl);
    downloader->curl = nullptr;
    downloader->status = DOWNLOAD_STAGE_EXTRACTING;

    // Close the file handle
    std::fclose(downloader->temp_file_handle);

    // Open, but reading this time
    downloader->temp_file_handle = std::fopen(downloader->temp_file.data(), "rb");
    if(!downloader->temp_file_handle) {
        downloader->status = DOWNLOAD_STAGE_FAILED;
        downloader->mutex.unlock();
        return;
    }

    // Unlock the mutex
    downloader->mutex.unlock();

    #else

    // If we're just testing extraction, pretend everything just got finished
    downloader->mutex.lock();
    downloader->status = DOWNLOAD_STAGE_EXTRACTING;

    downloader->temp_file_handle = std::fopen(downloader->temp_file.data(), "rb");
    if(!downloader->temp_file_handle) {
        downloader->status = DOWNLOAD_STAGE_FAILED;
        downloader->mutex.unlock();
        return;
    }
    downloader->mutex.unlock();

    #endif
}

// Callback class
class HACMapDownloader::HACMapDownloaderCallback {
public:
    // When we've received data, put it in here
    static size_t write_callback(std::byte *ptr, std::size_t, std::size_t nmemb, HACMapDownloader *userdata) {
        userdata->mutex.lock();
        userdata->status = HACMapDownloader::DOWNLOAD_STAGE_DOWNLOADING;
        std::fwrite(ptr, nmemb, 1, userdata->temp_file_handle);
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

// Set up stuff
void HACMapDownloader::dispatch() {
    // Lock the mutex
    this->mutex.lock();
    if(this->curl) {
        this->mutex.unlock();
        std::terminate();
        return;
    }

    #ifndef TEST_EXTRACTION_ONLY
    this->temp_file_handle = std::fopen(this->temp_file.data(), "wb");

    // If we failed to open, give up and close, unlocking the mutex
    if(!this->temp_file_handle) {
        this->status = HACMapDownloader::DOWNLOAD_STAGE_FAILED;
        this->mutex.unlock();
        return;
    }

    // Initialize cURL as well as the downloader variables
    this->curl = curl_easy_init();

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
    #endif

    // Set the download stage to starting
    this->status = HACMapDownloader::DOWNLOAD_STAGE_STARTING;

    // Unlock
    this->mutex.unlock();

    std::thread(HACMapDownloader::dispatch_thread, this).detach();
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

bool HACMapDownloader::is_finished() noexcept {
    auto m = this->mutex.try_lock();
    if(!m) {
        return false;
    }
    bool finished = this->status == DOWNLOAD_STAGE_COMPLETE || this->status == DOWNLOAD_STAGE_FAILED || this->status == DOWNLOAD_STAGE_NOT_STARTED;
    this->mutex.unlock();
    return finished;
}

HACMapDownloader::HACMapDownloader(const char *map, const char *temp_file, const char *map_file) : map(map), temp_file(temp_file), map_file(map_file) {}
HACMapDownloader::~HACMapDownloader() {
    while(!this->is_finished());
}
