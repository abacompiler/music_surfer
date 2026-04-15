#include "utils/MetadataParser.hpp"

#include <chrono>
#include <string>

#if MUSIC_SURFER_HAS_TAGLIB
#include <taglib/fileref.h>
#include <taglib/tag.h>
#endif

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

    std::string title = stem;
#if MUSIC_SURFER_HAS_TAGLIB
    std::string artist = "Unknown Artist";
    std::string album = "Unknown Album";
#else
    std::string artist = "unknown";
    std::string album = "unknown";
#endif
    std::chrono::milliseconds duration{0};

#if MUSIC_SURFER_HAS_TAGLIB
    const TagLib::FileRef fileRef(filePath.c_str());
    if (!fileRef.isNull() && fileRef.tag() != nullptr)
    {
        const TagLib::Tag* tag = fileRef.tag();
        const std::string parsedTitle = tag->title().to8Bit(true);
        const std::string parsedArtist = tag->artist().to8Bit(true);
        const std::string parsedAlbum = tag->album().to8Bit(true);

        if (!parsedTitle.empty())
        {
            title = parsedTitle;
        }
        if (!parsedArtist.empty())
        {
            artist = parsedArtist;
        }
        if (!parsedAlbum.empty())
        {
            album = parsedAlbum;
        }
    }

    if (!fileRef.isNull() && fileRef.audioProperties() != nullptr)
    {
        const int seconds = fileRef.audioProperties()->lengthInSeconds();
        if (seconds > 0)
        {
            duration = std::chrono::seconds(seconds);
        }
    }
#endif

    core::Track track{"track:" + stem, title, "artist:" + artist, "album:" + album};
    track.setDuration(duration);
    return track;
}
} // namespace music_surfer::utils
