#pragma once

#include "core/Identifiers.hpp"

#include <string>
#include <vector>

namespace music_surfer::core
{
/**
 * @brief User playlist with immutable identity and ordered track references.
 *
 * Lifecycle: playlist entries are non-owning track IDs; higher-level managers
 * control storage and lifetime using RAII containers and smart pointers.
 */
class Playlist
{
public:
    /** @brief Construct a playlist with immutable ID and mutable name. */
    Playlist(PlaylistId id, std::string name);

    /** @brief Return immutable playlist identifier. */
    const PlaylistId& id() const noexcept;
    /** @brief Return playlist display name. */
    const std::string& name() const noexcept;
    /** @brief Return ordered track IDs in this playlist. */
    const std::vector<TrackId>& trackIds() const noexcept;

    /** @brief Update playlist display name. */
    void setName(std::string name);
    /** @brief Append a track reference to the playlist order. */
    void appendTrack(TrackId trackId);

private:
    const PlaylistId id_;
    std::string name_;
    std::vector<TrackId> trackIds_;
};
} // namespace music_surfer::core
