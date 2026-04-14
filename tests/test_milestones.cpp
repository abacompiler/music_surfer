#include "core/Track.hpp"
#include "services/Abstractions.hpp"
#include "services/AudioPlayer.hpp"
#include "services/LibraryManager.hpp"
#include "ui/MainWindow.hpp"
#include "utils/FileSystemScanner.hpp"
#include "utils/MetadataParser.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>

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
    (void)stats;
    assert(stats.filesDiscovered == 2);
    assert(stats.parsedTracks == 2);
    assert(stats.persistedTracks == 2);
    assert(repository->size() == 2);

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

void testUiPanelBindings()
{
    music_surfer::services::LibraryManager manager;
    music_surfer::services::AudioPlayer player;

    music_surfer::ui::MainWindow window;
    window.bindServices(&manager, &player);

    assert(window.trackListPanel().libraryManager == &manager);
    assert(window.playlistPanel().libraryManager == &manager);
    assert(window.transportPanel().audioPlayer == &player);
}
} // namespace

int main()
{
    testScannerAndParserIntegration();
    testLibraryManagerSyncFlow();
    testAudioTransportControls();
    testUiPanelBindings();
    return 0;
}
