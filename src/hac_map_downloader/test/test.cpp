#include <cstdio>
#include "../hac_map_downloader.hpp"

int main(int argc, const char **argv) {
    if(argc != 4) {
        std::printf("Usage: %s <map name> <tmp path> <final path>\n", argv[0]);
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
                std::printf("Progress: %7.02f / %7.02f MiB (%4.01f%%)\r", dlnow / 1024.0F / 1024.0F, dltotal / 1024.0F / 1024.0F, static_cast<float>(dlnow) / dltotal * 100.0F);
                break;
            }
            case HACMapDownloader::DOWNLOAD_STAGE_EXTRACTING: {
                std::printf("%-80s\r", "Extracting...");
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
