#pragma once

#include "core/Playlist.hpp"

#include <optional>
#include <vector>

namespace music_surfer::services
{
/**
 * @brief Service interface for playlist lifecycle and track ordering operations.
 *
 * Lifecycle: implementations should be injected and owned via smart pointers,
 * with all persistence delegated through interfaces rather than globals.
 */
class PlaylistManager
{
public:
    virtual ~PlaylistManager() = default;

    /** @brief Create a new playlist and return its immutable ID. */
    virtual core::PlaylistId createPlaylist(const std::string& name) = 0;
    /** @brief Append a track reference to a playlist. */
    virtual void addTrack(const core::PlaylistId& playlistId, const core::TrackId& trackId) = 0;
    /** @brief Remove all occurrences of a track from a playlist. */
    virtual void removeTrack(const core::PlaylistId& playlistId, const core::TrackId& trackId) = 0;

    /** @brief Load a playlist by immutable ID. */
    virtual std::optional<core::Playlist> findPlaylist(const core::PlaylistId& id) const = 0;
    /** @brief Return all known playlists. */
    virtual std::vector<core::Playlist> listPlaylists() const = 0;
};
} // namespace music_surfer::services
