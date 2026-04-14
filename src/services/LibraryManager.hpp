#pragma once

#include "core/Album.hpp"
#include "core/Artist.hpp"
#include "core/Track.hpp"
#include "services/Abstractions.hpp"

#include <memory>
#include <optional>

namespace music_surfer::services
{
/**
 * @brief Service interface for coordinating library ingest and retrieval.
 *
 * Lifecycle: implementations are long-lived RAII objects created by composition
 * roots and wired with abstraction dependencies through smart pointers.
 */
class LibraryManager
{
public:
    virtual ~LibraryManager() = default;

    /** @brief Attach repository dependency used for library persistence. */
    virtual void setRepository(std::shared_ptr<ITrackRepository> repository) = 0;
    /** @brief Attach file scanner used for local file discovery. */
    virtual void setFileScanner(std::shared_ptr<IFileScanner> scanner) = 0;
    /** @brief Attach metadata reader used for file tag extraction. */
    virtual void setMetadataReader(std::shared_ptr<IMetadataReader> reader) = 0;

    /** @brief Add or update an artist aggregate in the library catalog. */
    virtual void upsertArtist(const core::Artist& artist) = 0;
    /** @brief Add or update an album aggregate in the library catalog. */
    virtual void upsertAlbum(const core::Album& album) = 0;
    /** @brief Add or update a track aggregate in the library catalog. */
    virtual void upsertTrack(const core::Track& track) = 0;

    /** @brief Retrieve a known track by immutable ID. */
    virtual std::optional<core::Track> findTrack(const core::TrackId& id) const = 0;
};
} // namespace music_surfer::services
