#include "utils/MetadataParser.hpp"

#include <string>

namespace music_surfer::utils
{
std::optional<core::Track> MetadataParser::parseTrack(const std::filesystem::path& filePath) const
{
    if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
    {
        return std::nullopt;
    }

    const std::string stem = filePath.stem().string();
    if (stem.empty())
    {
        return std::nullopt;
    }

    // Real TagLib extraction can replace these fallback defaults.
    core::Track track{"track:" + stem, stem, "artist:unknown", "album:unknown"};
    return track;
}
} // namespace music_surfer::utils
