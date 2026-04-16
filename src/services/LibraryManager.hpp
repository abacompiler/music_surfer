#pragma once

#include "core/Album.hpp"
#include "core/Artist.hpp"
#include "core/Track.hpp"
#include "services/Abstractions.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace music_surfer::services
{
/**
 * @brief Default service for coordinating library ingest and retrieval.
 *
 * Lifecycle: long-lived RAII object created by composition roots and wired with
 * abstraction dependencies through smart pointers.
 */
class LibraryManager
{
public:
    struct SyncStats
    {
        std::size_t filesDiscovered{0};
        std::size_t parsedTracks{0};
        std::size_t persistedTracks{0};
    };

    LibraryManager() = default;

    /** @brief Attach repository dependency used for library persistence. */
    void setRepository(std::shared_ptr<ITrackRepository> repository);
    /** @brief Attach file scanner used for local file discovery. */
    void setFileScanner(std::shared_ptr<IFileScanner> scanner);
    /** @brief Attach metadata reader used for file tag extraction. */
    void setMetadataReader(std::shared_ptr<IMetadataReader> reader);

    /** @brief Add or update an artist aggregate in the library catalog. */
    void upsertArtist(const core::Artist& artist);
    /** @brief Add or update an album aggregate in the library catalog. */
    void upsertAlbum(const core::Album& album);
    /** @brief Add or update a track aggregate in the library catalog. */
    void upsertTrack(const core::Track& track);

    /** @brief Retrieve a known track by immutable ID. */
    std::optional<core::Track> findTrack(const core::TrackId& id) const;
    /** @brief Return all known tracks currently in memory. */
    std::vector<core::Track> listTracks() const;

    /** @brief Scan files and persist parsed tracks through the repository. */
    SyncStats syncLibrary(const std::filesystem::path& root);

private:
    std::shared_ptr<ITrackRepository> repository_;
    std::shared_ptr<IFileScanner> scanner_;
    std::shared_ptr<IMetadataReader> reader_;

    std::unordered_map<core::ArtistId, core::Artist> artists_;
    std::unordered_map<core::AlbumId, core::Album> albums_;
    std::unordered_map<core::TrackId, core::Track> tracks_;
};
} // namespace music_surfer::services
