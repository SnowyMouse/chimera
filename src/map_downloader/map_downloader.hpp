// SPDX-License-Identifier: GPL-3.0-only

#ifndef MAP_DOWNLOADER_HPP
#define MAP_DOWNLOADER_HPP

#include <mutex>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <optional>
#include <filesystem>

/**
 * Map downloading class
 */
class MapDownloader {
public:
    enum DownloadStage {
        DOWNLOAD_STAGE_NOT_STARTED,
        DOWNLOAD_STAGE_STARTING,
        DOWNLOAD_STAGE_DOWNLOADING,
        DOWNLOAD_STAGE_COMPLETE,
        DOWNLOAD_STAGE_FAILED,
        DOWNLOAD_STAGE_CANCELING,
        DOWNLOAD_STAGE_CANCELED
    };

    /**
     * Begin the download
     */
    void download(const char *map, const char *output_file, const char *game_engine);

    /**
     * Abort the download
     */
    void cancel() noexcept;

    /**
     * Get the current download status
     * @return download status
     */
    DownloadStage get_status() noexcept;

    /**
     * Get the total downloaded size of the file.
     * @return file size bytes
     */
    std::size_t get_downloaded_size() noexcept;

    /**
     * Get the total size of the file. This can return 0 if the size is currently unknown.
     * @return file size bytes
     */
    std::size_t get_total_size() noexcept;

    /**
     * Get whether or not the download is finished
     * @return true if the download is finished
     */
    bool is_finished() noexcept;

    /**
     * Get the download speed in kilobytes per second
     * @return download speed in kilobytes per second
     */
    std::size_t get_download_speed() noexcept;

    /**
     * Get the map name
     * @return map name
     */
    const std::string &get_map() const noexcept;

    /**
     * Set the current server data
     * @param server   The IP/domain of the server
     * @param password The password for the server
     */
    void set_server_info(const std::string &server, const std::string &password) noexcept;

    MapDownloader(const std::string &url_template);
    ~MapDownloader();

private:
    /** Mutex for the rest of the data */
    std::mutex mutex;

    /** The format of the URL to download maps from **/
    std::string url_template;

    /** Map name being downloaded */
    std::string map;

    /** Server information */
    std::string server;
    std::string password;

    /** Temp file to save to */
    std::filesystem::path output_file;

    /** Post! */
    std::string post_fields;

    /** Game engine */
    std::string game_engine;

    /** File to write to as we download */
    std::FILE *output_file_handle = nullptr;

    /** How much was written to disk? */
    std::size_t written_size = 0;

    /** How much was downloaded so far */
    std::size_t downloaded_size = 0;

    /** How much is left to download */
    std::size_t total_size = 0;

    /** Current status of the download */
    DownloadStage status = DOWNLOAD_STAGE_NOT_STARTED;

    /** Buffer for holding data */
    std::vector<std::byte> buffer;

    /** Amount of the buffer used */
    std::size_t buffer_used;

    /** Clock to use */
    using Clock = std::chrono::steady_clock;

    /** Time when the download started*/
    Clock::time_point download_started;

    /** CURL handle */
    void *curl = nullptr;

    /** Callback class */
    class MapDownloaderCallback;

    /** Dispatch thread that does map downloading */
    std::thread dispatch_thread;

    /**
     * Dispatch thread function that does the map downloading
     * @param downloader downloader reference
     */
    static void dispatch_thread_function(MapDownloader *downloader);

    /**
     * Check if finished without locking
     * @return true if finished
     */
    bool is_finished_no_mutex() const noexcept;
};

#endif
