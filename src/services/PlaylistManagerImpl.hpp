#pragma once

#include "database/Database.hpp"
#include "services/PlaylistManager.hpp"

#include <memory>

namespace music_surfer::services
{
class PlaylistManagerImpl final : public PlaylistManager
{
public:
    explicit PlaylistManagerImpl(std::shared_ptr<database::Database> database);

    core::PlaylistId createPlaylist(const std::string& name) override;
    void addTrack(const core::PlaylistId& playlistId, const core::TrackId& trackId) override;
    void removeTrack(const core::PlaylistId& playlistId, const core::TrackId& trackId) override;

    std::optional<core::Playlist> findPlaylist(const core::PlaylistId& id) const override;
    std::vector<core::Playlist> listPlaylists() const override;

private:
    std::shared_ptr<database::Database> database_;
};
} // namespace music_surfer::services
