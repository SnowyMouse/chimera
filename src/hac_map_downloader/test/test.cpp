#include <cstdio>
#include "../hac_map_downloader.hpp"

int main(int argc, const char **argv) {
    if(argc != 4) {
        std::printf("Usage: %s <map name> <tmp path> <engine>\n", argv[0]);
        return EXIT_FAILURE;
    }

    HACMapDownloader downloader(argv[1], argv[2], argv[3]);
    downloader.dispatch();

    for(;;) {
        switch(downloader.get_status()) {
            case HACMapDownloader::DOWNLOAD_STAGE_COMPLETE: {
                break;
            }
            case HACMapDownloader::DOWNLOAD_STAGE_DOWNLOADING: {
                auto dlnow = downloader.get_downloaded_size();
                auto dltotal = downloader.get_total_size();

                char download_speed_buffer[32];

                auto download_speed = downloader.get_download_speed();
                if(download_speed > 1000) {
                    std::snprintf(download_speed_buffer, sizeof(download_speed_buffer), "%.01f MB/s", download_speed / 1000.0F);
                }
                else {
                    std::snprintf(download_speed_buffer, sizeof(download_speed_buffer), "%zu kB/s", download_speed);
                }

                char full_buffer[80];
                std::snprintf(full_buffer, sizeof(full_buffer), "Progress: %7.02f / %7.02f MiB (%4.01f%%, %s)", dlnow / 1024.0F / 1024.0F, dltotal / 1024.0F / 1024.0F, static_cast<float>(dlnow) / dltotal * 100.0F, download_speed_buffer);
                std::printf("%-80s\r", full_buffer);
                break;
            }
            case HACMapDownloader::DOWNLOAD_STAGE_STARTING: {
                std::printf("%-80s\r", "Connecting...");
                break;
            }
            default: {
                std::printf("%-80s\n", "Failed!");
                return EXIT_FAILURE;
            }
        }
        if(downloader.is_finished()) {
            break;
        }
    }

    std::printf("%-80s\n", "Done! Successfully downloaded the map!");
}
