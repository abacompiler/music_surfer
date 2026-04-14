#pragma once

#include "core/Identifiers.hpp"

#include <chrono>
#include <string>

namespace music_surfer::core
{
/**
 * @brief Immutable-identity value object for a library track.
 *
 * Lifecycle: owned by services via smart pointers; this type has no global state
 * and follows RAII through standard-library members only.
 */
class Track
{
public:
    /**
     * @brief Construct a track with an immutable ID and basic metadata.
     */
    Track(TrackId id, std::string title, ArtistId artistId, AlbumId albumId);

    /** @brief Return the immutable track identifier. */
    const TrackId& id() const noexcept;
    /** @brief Return the display title. */
    const std::string& title() const noexcept;
    /** @brief Return the owning artist identifier. */
    const ArtistId& artistId() const noexcept;
    /** @brief Return the owning album identifier. */
    const AlbumId& albumId() const noexcept;
    /** @brief Return track duration in milliseconds. */
    std::chrono::milliseconds duration() const noexcept;

    /** @brief Update mutable display title metadata. */
    void setTitle(std::string title);
    /** @brief Update mutable duration metadata. */
    void setDuration(std::chrono::milliseconds duration) noexcept;

private:
    const TrackId id_;
    std::string title_;
    ArtistId artistId_;
    AlbumId albumId_;
    std::chrono::milliseconds duration_{0};
};
} // namespace music_surfer::core
