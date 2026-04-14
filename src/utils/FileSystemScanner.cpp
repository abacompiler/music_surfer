#include "utils/FileSystemScanner.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace music_surfer::utils
{
FileSystemScanner::FileSystemScanner() : extensions_{".mp3", ".flac", ".wav", ".aac", ".ogg", ".m4a"} {}

std::vector<std::filesystem::path> FileSystemScanner::scanAudioFiles(const std::filesystem::path& root) const
{
    std::vector<std::filesystem::path> matches;
    if (!std::filesystem::exists(root))
    {
        return matches;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        std::string extension = entry.path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (extensions_.find(extension) != extensions_.end())
        {
            matches.push_back(entry.path());
        }
    }

    return matches;
}
} // namespace music_surfer::utils
