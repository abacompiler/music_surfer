#include "services/LibraryManager.hpp"

#include <utility>

namespace music_surfer::services
{
void LibraryManager::setRepository(std::shared_ptr<ITrackRepository> repository) { repository_ = std::move(repository); }

void LibraryManager::setFileScanner(std::shared_ptr<IFileScanner> scanner) { scanner_ = std::move(scanner); }

void LibraryManager::setMetadataReader(std::shared_ptr<IMetadataReader> reader) { reader_ = std::move(reader); }

void LibraryManager::upsertArtist(const core::Artist& artist)
{
    artists_.erase(artist.id());
    artists_.emplace(artist.id(), artist);
}

void LibraryManager::upsertAlbum(const core::Album& album)
{
    albums_.erase(album.id());
    albums_.emplace(album.id(), album);
}

void LibraryManager::upsertTrack(const core::Track& track)
{
    tracks_.erase(track.id());
    tracks_.emplace(track.id(), track);
    if (repository_)
    {
        repository_->saveTrack(track);
    }
}

std::optional<core::Track> LibraryManager::findTrack(const core::TrackId& id) const
{
    const auto local = tracks_.find(id);
    if (local != tracks_.end())
    {
        return local->second;
    }

    if (repository_)
    {
        return repository_->loadTrack(id);
    }

    return std::nullopt;
}

LibraryManager::SyncStats LibraryManager::syncLibrary(const std::filesystem::path& root)
{
    SyncStats stats;
    if (!scanner_ || !reader_)
    {
        return stats;
    }

    const auto files = scanner_->scanAudioFiles(root);
    stats.filesDiscovered = files.size();

    for (const auto& file : files)
    {
        auto parsedTrack = reader_->parseTrack(file);
        if (!parsedTrack)
        {
            continue;
        }

        ++stats.parsedTracks;
        upsertTrack(*parsedTrack);
        ++stats.persistedTracks;
    }

    return stats;
}
} // namespace music_surfer::services
