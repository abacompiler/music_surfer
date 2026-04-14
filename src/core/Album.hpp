#pragma once

#include "core/Identifiers.hpp"

#include <string>
#include <vector>

namespace music_surfer::core
{
/**
 * @brief Album aggregate with immutable identity and track membership.
 *
 * Lifecycle: tracks are referenced by ID to avoid ownership cycles; service
 * layer coordinates object ownership via smart pointers.
 */
class Album
{
public:
    /** @brief Construct an album with immutable ID and primary artist owner. */
    Album(AlbumId id, std::string title, ArtistId artistId);

    /** @brief Return immutable album identifier. */
    const AlbumId& id() const noexcept;
    /** @brief Return album display title. */
    const std::string& title() const noexcept;
    /** @brief Return owning artist identifier. */
    const ArtistId& artistId() const noexcept;
    /** @brief Return track IDs in album order. */
    const std::vector<TrackId>& trackIds() const noexcept;

    /** @brief Update mutable album title metadata. */
    void setTitle(std::string title);
    /** @brief Add a track ID to this album if missing. */
    void addTrack(TrackId trackId);

private:
    const AlbumId id_;
    std::string title_;
    ArtistId artistId_;
    std::vector<TrackId> trackIds_;
};
} // namespace music_surfer::core
