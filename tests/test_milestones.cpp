#include "core/Playlist.hpp"
#include "core/Track.hpp"
#include "services/Abstractions.hpp"
#include "services/AudioPlayer.hpp"
#include "services/LibraryManager.hpp"
#include "services/PlaylistManager.hpp"
#include "ui/MainWindow.hpp"
#include "utils/FileSystemScanner.hpp"
#include "utils/MetadataParser.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <utility>

namespace
{
class InMemoryRepository final : public music_surfer::services::ITrackRepository
{
public:
    void saveTrack(const music_surfer::core::Track& track) override
    {
        tracks_.erase(track.id());
        tracks_.emplace(track.id(), track);
    }

    std::optional<music_surfer::core::Track> loadTrack(const music_surfer::core::TrackId& id) const override
    {
        const auto it = tracks_.find(id);
        if (it == tracks_.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    std::size_t size() const noexcept { return tracks_.size(); }

private:
    std::unordered_map<music_surfer::core::TrackId, music_surfer::core::Track> tracks_;
};

class InMemoryPlaylistManager final : public music_surfer::services::PlaylistManager
{
public:
    music_surfer::core::PlaylistId createPlaylist(const std::string& name) override
    {
        const auto id = std::string("pl-") + std::to_string(records_.size() + 1);
        records_.push_back(PlaylistRecord{id, name, {}});
        return id;
    }

    void addTrack(const music_surfer::core::PlaylistId& playlistId, const music_surfer::core::TrackId& trackId) override
    {
        auto* record = findMutable(playlistId);
        if (record != nullptr)
        {
            record->trackIds.push_back(trackId);
        }
    }

    void removeTrack(const music_surfer::core::PlaylistId& playlistId, const music_surfer::core::TrackId& trackId) override
    {
        auto* record = findMutable(playlistId);
        if (record == nullptr)
        {
            return;
        }

        record->trackIds.erase(
            std::remove(record->trackIds.begin(), record->trackIds.end(), trackId), record->trackIds.end());
    }

    void deletePlaylist(const music_surfer::core::PlaylistId& playlistId) override
    {
        records_.erase(std::remove_if(records_.begin(),
                                      records_.end(),
                                      [&](const auto& record) { return record.id == playlistId; }),
                       records_.end());
    }

    std::optional<music_surfer::core::Playlist> findPlaylist(const music_surfer::core::PlaylistId& id) const override
    {
        for (const auto& record : records_)
        {
            if (record.id == id)
            {
                return toCorePlaylist(record);
            }
        }
        return std::nullopt;
    }

    std::vector<music_surfer::core::Playlist> listPlaylists() const override
    {
        std::vector<music_surfer::core::Playlist> playlists;
        playlists.reserve(records_.size());
        for (const auto& record : records_)
        {
            playlists.push_back(toCorePlaylist(record));
        }
        return playlists;
    }

private:
    struct PlaylistRecord
    {
        music_surfer::core::PlaylistId id;
        std::string name;
        std::vector<music_surfer::core::TrackId> trackIds;
    };

    static music_surfer::core::Playlist toCorePlaylist(const PlaylistRecord& record)
    {
        music_surfer::core::Playlist playlist(record.id, record.name);
        for (const auto& trackId : record.trackIds)
        {
            playlist.appendTrack(trackId);
        }
        return playlist;
    }

    PlaylistRecord* findMutable(const music_surfer::core::PlaylistId& playlistId)
    {
        for (auto& record : records_)
        {
            if (record.id == playlistId)
            {
                return &record;
            }
        }
        return nullptr;
    }

    std::vector<PlaylistRecord> records_;
};

std::filesystem::path makeTempRoot()
{
    const auto uniquePart = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = std::filesystem::temp_directory_path() / ("music_surfer_samples_" + uniquePart);
    std::filesystem::create_directories(root / "nested");
    return root;
}

void testScannerAndParserIntegration()
{
    const auto root = makeTempRoot();
    {
        std::ofstream(root / "song_a.mp3") << "sample-a";
        std::ofstream(root / "nested" / "song_b.flac") << "sample-b";
        std::ofstream(root / "ignore.txt") << "not audio";
    }

    music_surfer::utils::FileSystemScanner scanner;
    music_surfer::utils::MetadataParser parser;

    const auto files = scanner.scanAudioFiles(root);
    assert(files.size() == 2);

    std::size_t parsedCount = 0;
    for (const auto& file : files)
    {
        const auto parsed = parser.parseTrack(file);
        assert(parsed.has_value());
        ++parsedCount;
    }
    assert(parsedCount == 2);

    std::filesystem::remove_all(root);
}

void testMetadataFallbackBehavior()
{
    const auto root = makeTempRoot();
    const auto file = root / "nested" / "mystery_track.mp3";
    {
        std::ofstream(file) << "fake-audio-content";
    }

    music_surfer::utils::MetadataParser parser;
    const auto parsed = parser.parseTrack(file);
    assert(parsed.has_value());

#if MUSIC_SURFER_HAS_TAGLIB
    assert(parsed->title() == "mystery_track");
    assert(parsed->artistId() == "artist:Unknown Artist");
    assert(parsed->albumId() == "album:Unknown Album");
    assert(parsed->duration() == std::chrono::milliseconds::zero());
#else
    assert(parsed->title() == "mystery_track");
    assert(parsed->artistId() == "artist:unknown");
    assert(parsed->albumId() == "album:unknown");
    assert(parsed->duration() == std::chrono::milliseconds::zero());
#endif

    std::filesystem::remove_all(root);
}

void testLibraryManagerSyncFlow()
{
    const auto root = makeTempRoot();
    {
        std::ofstream(root / "track_one.mp3") << "sample-1";
        std::ofstream(root / "nested" / "track_two.ogg") << "sample-2";
    }

    auto repository = std::make_shared<InMemoryRepository>();
    auto scanner = std::make_shared<music_surfer::utils::FileSystemScanner>();
    auto parser = std::make_shared<music_surfer::utils::MetadataParser>();

    music_surfer::services::LibraryManager manager;
    manager.setRepository(repository);
    manager.setFileScanner(scanner);
    manager.setMetadataReader(parser);

    const auto stats = manager.syncLibrary(root);
    assert(stats.filesDiscovered == 2);
    assert(stats.parsedTracks == 2);
    assert(stats.persistedTracks == 2);
    assert(repository->size() == 2);
    assert(manager.listTracks().size() == 2);

    const auto fetched = manager.findTrack("track:track_one");
    assert(fetched.has_value());
    assert(fetched->title() == "track_one");

    std::filesystem::remove_all(root);
}

void testAudioTransportControls()
{
    music_surfer::core::Track track{"track-42", "Track 42", "artist-x", "album-y"};
    track.setDuration(std::chrono::milliseconds(10000));

    music_surfer::services::AudioPlayer player;
    player.load(track);
    assert(player.state() == music_surfer::services::AudioPlayer::State::Stopped);

    player.play();
    assert(player.state() == music_surfer::services::AudioPlayer::State::Playing);

    player.pause();
    assert(player.state() == music_surfer::services::AudioPlayer::State::Paused);

    player.seek(std::chrono::milliseconds(2500));
    assert(player.position() == std::chrono::milliseconds(2500));

    player.seek(std::chrono::milliseconds(20000));
    assert(player.position() == std::chrono::milliseconds(10000));

    player.stop();
    assert(player.state() == music_surfer::services::AudioPlayer::State::Stopped);
    assert(player.position() == std::chrono::milliseconds::zero());
}

void testUiPanelsAndPresenters()
{
    music_surfer::services::LibraryManager manager;
    music_surfer::services::AudioPlayer player;
    InMemoryPlaylistManager playlists;

    manager.upsertTrack(music_surfer::core::Track{"track-1", "Track One", "artist-1", "album-1"});

    music_surfer::ui::MainWindow window(manager, player, playlists);

    assert(window.trackListComponent().viewModel().tracks.size() == 1);

    window.trackListComponent().onTrackSelected("track-1");
    window.trackListComponent().onLoadTrackRequested();
    assert(player.currentTrack().has_value());
    assert(player.currentTrack()->id() == "track-1");

    window.playbackControlsComponent().onPlayPressed();
    assert(player.state() == music_surfer::services::AudioPlayer::State::Playing);

    window.playbackControlsComponent().onPausePressed();
    assert(player.state() == music_surfer::services::AudioPlayer::State::Paused);

    window.playbackControlsComponent().onStopPressed();
    assert(player.state() == music_surfer::services::AudioPlayer::State::Stopped);

    window.playlistPanelComponent().onCreatePlaylistRequested("Faves");
    auto playlistModels = window.playlistPanelComponent().viewModel().playlists;
    assert(playlistModels.size() == 1);

    const auto playlistId = playlistModels.front().id;
    window.playlistPanelComponent().onAddTrackRequested(playlistId, "track-1");
    const auto createdPlaylistAfterAdd = playlists.findPlaylist(playlistId);
    assert(createdPlaylistAfterAdd.has_value());
    assert(createdPlaylistAfterAdd->trackIds().size() == 1);

    window.playlistPanelComponent().onRemoveTrackRequested(playlistId, "track-1");
    const auto createdPlaylistAfterRemove = playlists.findPlaylist(playlistId);
    assert(createdPlaylistAfterRemove.has_value());
    assert(createdPlaylistAfterRemove->trackIds().empty());

    window.playlistPanelComponent().onDeletePlaylistRequested(playlistId);
    assert(playlists.listPlaylists().empty());
}
} // namespace

int main()
{
    testScannerAndParserIntegration();
    testMetadataFallbackBehavior();
    testLibraryManagerSyncFlow();
    testAudioTransportControls();
    testUiPanelsAndPresenters();
    return 0;
}
