#ifndef HAC_MAP_DOWNLOADER_HPP
#define HAC_MAP_DOWNLOADER_HPP

#include <mutex>
#include <vector>
#include <cstdlib>

/**
 * Map downloading class
 */
class HACMapDownloader {
public:
    /**
     * Begin the download
     */
    void dispatch();

    /**
     * Check if the download is complete. If it is complete, use is_successful() to check if no errors occurred.
     * @return true if the download is complete
     */
    bool is_finished() noexcept;

    /**
     * Check if the map was successfully downloaded.
     * @return true if the map was successfully downloaded
     */
    bool is_successful() noexcept;

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
     * Get a vector holding all of the map data
     * @return pointer to a vector holding map data or nullptr if the download was not successful
     */
    const std::vector<std::byte> *get_map_data() noexcept;

    HACMapDownloader(const char *map, const char *temp_file, const char *map_file);
    ~HACMapDownloader();

private:
    /** Mutex for the rest of the data */
    std::mutex mutex;

    /** Map name being downloaded */
    std::string map;

    /** Temp file to save to */
    std::string temp_file;

    /** Map file to save to */
    std::string map_file;

    /** Post! */
    std::string post_fields;

    /** Are we done? */
    bool finished = true;

    /** Did it work? */
    bool successful = false;

    /** File to write to as we download */
    std::FILE *temp_file_handle;

    /** How much was downloaded so far */
    std::size_t downloaded_size = 0;

    /** How much is left to download */
    std::size_t total_size = 0;

    /**
     * Dispatch thread that does the map downloading
     * @param downloader downloader reference
     */
    static void dispatch_thread(HACMapDownloader *downloader);

    /** CURL handle */
    void *curl = nullptr;

    /** Lock the mutex */
    void lock();

    /**
     * Try to lock the mutex
     * @return true if locked
     */
    bool try_lock();

    /** Unlock the mutex */
    void unlock();

    /** Data to download */
    std::vector<std::byte> compressed_data;

    /** Data decompressed */
    std::vector<std::byte> decompressed_data;

    /** Callback class */
    class HACMapDownloaderCallback;
};

#endif
