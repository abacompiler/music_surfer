#include "database/Database.hpp"

#include <utility>

#if MUSIC_SURFER_HAS_SQLITE
#include <sqlite3.h>
#endif

namespace music_surfer::database
{
struct Database::Connection
{
#if MUSIC_SURFER_HAS_SQLITE
    sqlite3* handle{nullptr};
#endif
    std::filesystem::path filePath;
};

namespace
{
#if MUSIC_SURFER_HAS_SQLITE
bool executeSql(sqlite3* handle, const char* sql)
{
    return sqlite3_exec(handle, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
}

std::optional<TrackRecord> readTrack(sqlite3_stmt* statement)
{
    TrackRecord record;
    record.id = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
    record.title = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
    record.artist = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
    record.album = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
    record.path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
    record.duration = std::chrono::milliseconds(sqlite3_column_int64(statement, 5));
    return record;
}
#endif
} // namespace

Database::Database() = default;
Database::Database(const std::filesystem::path& databasePath) { (void)connect(databasePath); }
Database::~Database() { disconnect(); }
Database::Database(Database&&) noexcept = default;
Database& Database::operator=(Database&&) noexcept = default;

bool Database::connect(const std::filesystem::path& databasePath)
{
    disconnect();

#if MUSIC_SURFER_HAS_SQLITE
    auto connection = std::make_unique<Connection>();
    connection->filePath = databasePath;

    if (sqlite3_open(databasePath.string().c_str(), &connection->handle) != SQLITE_OK)
    {
        if (connection->handle != nullptr)
        {
            sqlite3_close(connection->handle);
            connection->handle = nullptr;
        }
        return false;
    }

    if (!executeSql(connection->handle, "PRAGMA foreign_keys = ON;"))
    {
        sqlite3_close(connection->handle);
        connection->handle = nullptr;
        return false;
    }

    connection_ = std::move(connection);
    if (!initializeSchema())
    {
        disconnect();
        return false;
    }
    return true;
#else
    (void)databasePath;
    return false;
#endif
}

void Database::disconnect() noexcept
{
#if MUSIC_SURFER_HAS_SQLITE
    if (connection_ && connection_->handle != nullptr)
    {
        sqlite3_close(connection_->handle);
        connection_->handle = nullptr;
    }
#endif
    connection_.reset();
}

bool Database::isConnected() const noexcept { return static_cast<bool>(connection_); }

bool Database::initializeSchema()
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return false;
    }

    static constexpr const char* kSchema =
        "CREATE TABLE IF NOT EXISTS Tracks("
        "id TEXT PRIMARY KEY,"
        "title TEXT NOT NULL,"
        "artist TEXT NOT NULL,"
        "album TEXT NOT NULL,"
        "path TEXT NOT NULL UNIQUE,"
        "duration INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS Playlists("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS PlaylistTracks("
        "playlist_id TEXT NOT NULL,"
        "track_id TEXT NOT NULL,"
        "PRIMARY KEY(playlist_id, track_id),"
        "FOREIGN KEY(playlist_id) REFERENCES Playlists(id) ON DELETE CASCADE,"
        "FOREIGN KEY(track_id) REFERENCES Tracks(id) ON DELETE CASCADE"
        ");";

    return executeSql(connection_->handle, kSchema);
#else
    return false;
#endif
}

void Database::saveTrack(const core::Track& track)
{
    TrackRecord record{track.id(), track.title(), track.artistId(), track.albumId(), track.id(), track.duration()};
    (void)upsertTrackByPath(record);
}

std::optional<core::Track> Database::loadTrack(const core::TrackId& id) const
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return std::nullopt;
    }

    static constexpr const char* kSql =
        "SELECT id, title, artist, album, duration FROM Tracks WHERE id = ?1 LIMIT 1;";
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        auto trackId = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)));
        auto title = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
        auto artist = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        auto album = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 3)));
        auto duration = std::chrono::milliseconds(sqlite3_column_int64(statement, 4));

        core::Track loadedTrack(trackId, title, artist, album);
        loadedTrack.setDuration(duration);
        sqlite3_finalize(statement);
        return loadedTrack;
    }

    sqlite3_finalize(statement);
    return std::nullopt;
#else
    (void)id;
    return std::nullopt;
#endif
}

bool Database::upsertTrackByPath(const TrackRecord& track)
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return false;
    }

    static constexpr const char* kSql =
        "INSERT INTO Tracks(id, title, artist, album, path, duration) "
        "VALUES(?1, ?2, ?3, ?4, ?5, ?6) "
        "ON CONFLICT(path) DO UPDATE SET "
        "id=excluded.id, "
        "title=excluded.title, "
        "artist=excluded.artist, "
        "album=excluded.album, "
        "duration=excluded.duration;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(statement, 1, track.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, track.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, track.artist.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, track.album.c_str(), -1, SQLITE_TRANSIENT);
    const auto pathString = track.path.string();
    sqlite3_bind_text(statement, 5, pathString.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 6, track.duration.count());

    const auto result = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return result;
#else
    (void)track;
    return false;
#endif
}

std::optional<TrackRecord> Database::getTrackByPath(const std::filesystem::path& path) const
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return std::nullopt;
    }

    static constexpr const char* kSql =
        "SELECT id, title, artist, album, path, duration FROM Tracks WHERE path = ?1 LIMIT 1;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return std::nullopt;
    }

    const auto pathString = path.string();
    sqlite3_bind_text(statement, 1, pathString.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<TrackRecord> track;
    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        track = readTrack(statement);
    }

    sqlite3_finalize(statement);
    return track;
#else
    (void)path;
    return std::nullopt;
#endif
}

std::vector<TrackRecord> Database::listTracks() const
{
    std::vector<TrackRecord> tracks;
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return tracks;
    }

    static constexpr const char* kSql =
        "SELECT id, title, artist, album, path, duration FROM Tracks ORDER BY title ASC;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return tracks;
    }

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        auto track = readTrack(statement);
        if (track.has_value())
        {
            tracks.push_back(std::move(*track));
        }
    }

    sqlite3_finalize(statement);
#endif
    return tracks;
}

bool Database::createPlaylist(const core::PlaylistId& playlistId, const std::string& name)
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return false;
    }

    static constexpr const char* kSql = "INSERT INTO Playlists(id, name) VALUES(?1, ?2);";
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(statement, 1, playlistId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, name.c_str(), -1, SQLITE_TRANSIENT);

    const bool result = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return result;
#else
    (void)playlistId;
    (void)name;
    return false;
#endif
}

bool Database::deletePlaylist(const core::PlaylistId& playlistId)
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return false;
    }

    static constexpr const char* kSql = "DELETE FROM Playlists WHERE id = ?1;";
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(statement, 1, playlistId.c_str(), -1, SQLITE_TRANSIENT);

    const bool result = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return result;
#else
    (void)playlistId;
    return false;
#endif
}

bool Database::addTrackToPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return false;
    }

    static constexpr const char* kSql =
        "INSERT OR IGNORE INTO PlaylistTracks(playlist_id, track_id) VALUES(?1, ?2);";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(statement, 1, playlistId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, trackId.c_str(), -1, SQLITE_TRANSIENT);

    const bool result = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return result;
#else
    (void)playlistId;
    (void)trackId;
    return false;
#endif
}

bool Database::removeTrackFromPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return false;
    }

    static constexpr const char* kSql = "DELETE FROM PlaylistTracks WHERE playlist_id = ?1 AND track_id = ?2;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kSql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(statement, 1, playlistId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, trackId.c_str(), -1, SQLITE_TRANSIENT);

    const bool result = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return result;
#else
    (void)playlistId;
    (void)trackId;
    return false;
#endif
}

std::vector<PlaylistRecord> Database::listPlaylists() const
{
    std::vector<PlaylistRecord> playlists;
#if MUSIC_SURFER_HAS_SQLITE
    if (!connection_ || connection_->handle == nullptr)
    {
        return playlists;
    }

    static constexpr const char* kPlaylistSql = "SELECT id, name FROM Playlists ORDER BY name ASC;";
    sqlite3_stmt* playlistsStatement = nullptr;
    if (sqlite3_prepare_v2(connection_->handle, kPlaylistSql, -1, &playlistsStatement, nullptr) != SQLITE_OK)
    {
        return playlists;
    }

    while (sqlite3_step(playlistsStatement) == SQLITE_ROW)
    {
        PlaylistRecord playlist;
        playlist.id = reinterpret_cast<const char*>(sqlite3_column_text(playlistsStatement, 0));
        playlist.name = reinterpret_cast<const char*>(sqlite3_column_text(playlistsStatement, 1));
        playlists.push_back(std::move(playlist));
    }
    sqlite3_finalize(playlistsStatement);

    static constexpr const char* kTracksSql =
        "SELECT track_id FROM PlaylistTracks WHERE playlist_id = ?1 ORDER BY rowid ASC;";

    for (auto& playlist : playlists)
    {
        sqlite3_stmt* tracksStatement = nullptr;
        if (sqlite3_prepare_v2(connection_->handle, kTracksSql, -1, &tracksStatement, nullptr) != SQLITE_OK)
        {
            continue;
        }

        sqlite3_bind_text(tracksStatement, 1, playlist.id.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(tracksStatement) == SQLITE_ROW)
        {
            playlist.trackIds.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(tracksStatement, 0)));
        }

        sqlite3_finalize(tracksStatement);
    }
#endif
    return playlists;
}
} // namespace music_surfer::database
