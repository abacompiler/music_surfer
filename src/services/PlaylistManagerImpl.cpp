#include "services/PlaylistManagerImpl.hpp"

#include <cctype>
#include <cstdint>
#include <unordered_set>
#include <utility>

namespace music_surfer::services
{
namespace
{
std::string normalizeNameForId(const std::string& name)
{
    std::string normalized;
    normalized.reserve(name.size());

    bool previousWasDash = false;
    for (const char rawCharacter : name)
    {
        const auto character = static_cast<unsigned char>(rawCharacter);
        if (std::isalnum(character) != 0)
        {
            normalized.push_back(static_cast<char>(std::tolower(character)));
            previousWasDash = false;
            continue;
        }

        if (!previousWasDash)
        {
            normalized.push_back('-');
            previousWasDash = true;
        }
    }

    while (!normalized.empty() && normalized.front() == '-')
    {
        normalized.erase(normalized.begin());
    }

    while (!normalized.empty() && normalized.back() == '-')
    {
        normalized.pop_back();
    }

    if (normalized.empty())
    {
        normalized = "playlist";
    }

    return normalized;
}

std::uint64_t fnv1a64(const std::string& value)
{
    constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ull;
    constexpr std::uint64_t kFnvPrime = 1099511628211ull;

    std::uint64_t hash = kFnvOffsetBasis;
    for (const char rawCharacter : value)
    {
        const auto character = static_cast<unsigned char>(rawCharacter);
        hash ^= character;
        hash *= kFnvPrime;
    }

    return hash;
}

std::string toPlaylistId(const std::string& normalizedName, std::size_t attempt)
{
    const std::string saltedName = attempt == 0 ? normalizedName : normalizedName + "#" + std::to_string(attempt);
    return "pl-" + std::to_string(fnv1a64(saltedName));
}

core::Playlist toCorePlaylist(const database::PlaylistRecord& record)
{
    core::Playlist playlist(record.id, record.name);
    for (const auto& trackId : record.trackIds)
    {
        playlist.appendTrack(trackId);
    }
    return playlist;
}
} // namespace

PlaylistManagerImpl::PlaylistManagerImpl(std::shared_ptr<database::Database> database) : database_(std::move(database)) {}

core::PlaylistId PlaylistManagerImpl::createPlaylist(const std::string& name)
{
    if (!database_)
    {
        return {};
    }

    const auto existingPlaylists = database_->listPlaylists();
    std::unordered_set<core::PlaylistId> existingIds;
    existingIds.reserve(existingPlaylists.size());
    for (const auto& playlist : existingPlaylists)
    {
        existingIds.insert(playlist.id);
    }

    const std::string normalizedName = normalizeNameForId(name);
    std::size_t attempt = 0;
    auto playlistId = toPlaylistId(normalizedName, attempt);
    while (existingIds.find(playlistId) != existingIds.end())
    {
        ++attempt;
        playlistId = toPlaylistId(normalizedName, attempt);
    }

    if (!database_->createPlaylist(playlistId, name))
    {
        return {};
    }

    return playlistId;
}

void PlaylistManagerImpl::addTrack(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
    if (!database_)
    {
        return;
    }

    (void)database_->addTrackToPlaylist(playlistId, trackId);
}

void PlaylistManagerImpl::removeTrack(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
    if (!database_)
    {
        return;
    }

    (void)database_->removeTrackFromPlaylist(playlistId, trackId);
}

void PlaylistManagerImpl::deletePlaylist(const core::PlaylistId& playlistId)
{
    if (!database_)
    {
        return;
    }

    (void)database_->deletePlaylist(playlistId);
}

std::optional<core::Playlist> PlaylistManagerImpl::findPlaylist(const core::PlaylistId& id) const
{
    if (!database_)
    {
        return std::nullopt;
    }

    const auto playlists = database_->listPlaylists();
    for (const auto& playlist : playlists)
    {
        if (playlist.id == id)
        {
            return toCorePlaylist(playlist);
        }
    }

    return std::nullopt;
}

std::vector<core::Playlist> PlaylistManagerImpl::listPlaylists() const
{
    std::vector<core::Playlist> playlists;
    if (!database_)
    {
        return playlists;
    }

    const auto records = database_->listPlaylists();
    playlists.reserve(records.size());
    for (const auto& record : records)
    {
        playlists.push_back(toCorePlaylist(record));
    }

    return playlists;
}
} // namespace music_surfer::services
