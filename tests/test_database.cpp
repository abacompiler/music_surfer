#include "database/Database.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <string>

namespace
{
std::filesystem::path testDatabasePath(const std::string& suffix)
{
    const auto uniquePart = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    return std::filesystem::temp_directory_path() / ("music_surfer_" + suffix + "_" + uniquePart + ".db");
}

void testTrackUpsertAndQuery()
{
    const auto dbPath = testDatabasePath("tracks");
    music_surfer::database::Database database(dbPath);
    assert(database.isConnected());

    music_surfer::database::TrackRecord inserted{"track-1",
                                                 "Original Title",
                                                 "Artist A",
                                                 "Album A",
                                                 "/library/track-a.mp3",
                                                 std::chrono::milliseconds(101000)};

    assert(database.upsertTrackByPath(inserted));

    auto fetched = database.getTrackByPath(inserted.path);
    assert(fetched.has_value());
    assert(fetched->id == "track-1");
    assert(fetched->title == "Original Title");

    music_surfer::database::TrackRecord updated{"track-1b",
                                                "Updated Title",
                                                "Artist A",
                                                "Album B",
                                                inserted.path,
                                                std::chrono::milliseconds(120000)};

    assert(database.upsertTrackByPath(updated));

    auto refetched = database.getTrackByPath(inserted.path);
    assert(refetched.has_value());
    assert(refetched->id == "track-1b");
    assert(refetched->title == "Updated Title");
    assert(refetched->album == "Album B");

    auto allTracks = database.listTracks();
    assert(allTracks.size() == 1);

    database.disconnect();
    std::filesystem::remove(dbPath);
}

void testPlaylistPersistenceAndRelations()
{
    const auto dbPath = testDatabasePath("playlists");
    music_surfer::database::Database database(dbPath);
    assert(database.isConnected());

    music_surfer::database::TrackRecord track{"track-foreign",
                                               "Foreign Key Track",
                                               "Artist",
                                               "Album",
                                               "/library/track-b.mp3",
                                               std::chrono::milliseconds(90000)};
    assert(database.upsertTrackByPath(track));

    assert(database.createPlaylist("playlist-1", "Faves"));
    assert(database.addTrackToPlaylist("playlist-1", "track-foreign"));

    auto playlists = database.listPlaylists();
    assert(playlists.size() == 1);
    assert(playlists.front().id == "playlist-1");
    assert(playlists.front().trackIds.size() == 1);
    assert(playlists.front().trackIds.front() == "track-foreign");

    assert(!database.addTrackToPlaylist("playlist-1", "missing-track"));

    assert(database.removeTrackFromPlaylist("playlist-1", "track-foreign"));
    playlists = database.listPlaylists();
    assert(playlists.size() == 1);
    assert(playlists.front().trackIds.empty());

    assert(database.addTrackToPlaylist("playlist-1", "track-foreign"));
    assert(database.deletePlaylist("playlist-1"));
    playlists = database.listPlaylists();
    assert(playlists.empty());

    database.disconnect();
    std::filesystem::remove(dbPath);
}
} // namespace

int main()
{
    testTrackUpsertAndQuery();
    testPlaylistPersistenceAndRelations();
    return 0;
}
