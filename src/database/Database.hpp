#pragma once

#include "core/Playlist.hpp"
#include "services/Abstractions.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace music_surfer::database
{
struct TrackRecord
{
    core::TrackId id;
    std::string title;
    std::string artist;
    std::string album;
    std::filesystem::path path;
    std::chrono::milliseconds duration{0};
};

struct PlaylistRecord
{
    core::PlaylistId id;
    std::string name;
    std::vector<core::TrackId> trackIds;
};

/**
 * @brief SQLite-backed repository wrapper implementing service storage abstraction.
 *
 * Lifecycle: owns the database handle through RAII and is intended to be
 * injected as an `ITrackRepository` dependency; no global connection is used.
 */
class Database final : public services::ITrackRepository
{
public:
    /** @brief Construct a disconnected repository wrapper. */
    Database();
    /** @brief Construct and connect to the provided SQLite database path. */
    explicit Database(const std::filesystem::path& databasePath);
    /** @brief Close any open connection and release native resources. */
    ~Database() override;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;

    /** @brief Open a SQLite database file and initialize schema if needed. */
    bool connect(const std::filesystem::path& databasePath);
    /** @brief Explicitly close the SQLite connection. */
    void disconnect() noexcept;
    /** @brief Report whether the wrapper currently has an open connection. */
    bool isConnected() const noexcept;

    /** @brief Persist a track to storage (no-op when disconnected). */
    void saveTrack(const core::Track& track) override;
    /** @brief Load a track by ID, if available in storage. */
    std::optional<core::Track> loadTrack(const core::TrackId& id) const override;

    /** @brief Insert or update track metadata using the file path as conflict key. */
    bool upsertTrackByPath(const TrackRecord& track);
    /** @brief Query a track by exact library path. */
    std::optional<TrackRecord> getTrackByPath(const std::filesystem::path& path) const;
    /** @brief Fetch all persisted tracks. */
    std::vector<TrackRecord> listTracks() const;

    /** @brief Create a playlist row. */
    bool createPlaylist(const core::PlaylistId& playlistId, const std::string& name);
    /** @brief Delete a playlist row and dependent associations. */
    bool deletePlaylist(const core::PlaylistId& playlistId);
    /** @brief Associate a track with a playlist. */
    bool addTrackToPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId);
    /** @brief Remove a track association from a playlist. */
    bool removeTrackFromPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId);
    /** @brief Fetch all playlists, including associated track IDs. */
    std::vector<PlaylistRecord> listPlaylists() const;

private:
    bool initializeSchema();

    struct Connection;
    std::unique_ptr<Connection> connection_;
};
} // namespace music_surfer::database
