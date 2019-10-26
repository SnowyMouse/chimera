#include <cstdio>
#include "../hac_map_downloader.hpp"

int main(int argc, const char **argv) {
    if(argc != 5) {
        std::printf("Usage: %s <map name> <tmp path> <final path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    HACMapDownloader downloader(argv[1], argv[2], argv[3]);
    downloader.dispatch();

    while(!downloader.is_finished()) {
        auto dlnow = downloader.get_downloaded_size();
        auto dltotal = downloader.get_total_size();
        std::printf("Progress: %7.02f / %7.02f MiB (%4.01f%%)\r", dlnow / 1024.0F / 1024.0F, dltotal / 1024.0F / 1024.0F, static_cast<float>(dlnow) / dltotal * 100.0F);
    }

    if(downloader.is_successful()) {
        std::printf("%-80s\n", "Successfully downloaded the map!");
    }
    else {
        std::printf("%-80s\n", "Failed to download the map!");
    }
}
