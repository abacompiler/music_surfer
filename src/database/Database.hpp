#pragma once

#include "services/Abstractions.hpp"

#include <filesystem>
#include <memory>

namespace music_surfer::database
{
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

private:
    struct Connection;
    std::unique_ptr<Connection> connection_;
};
} // namespace music_surfer::database
