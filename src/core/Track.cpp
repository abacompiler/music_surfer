#include "core/Album.hpp"
#include "core/Artist.hpp"
#include "core/Playlist.hpp"
#include "core/Track.hpp"

#include <algorithm>
#include <utility>

namespace music_surfer::core
{
Track::Track(TrackId id, std::string title, ArtistId artistId, AlbumId albumId)
    : id_(std::move(id)), title_(std::move(title)), artistId_(std::move(artistId)), albumId_(std::move(albumId))
{
}

const TrackId& Track::id() const noexcept { return id_; }
const std::string& Track::title() const noexcept { return title_; }
const ArtistId& Track::artistId() const noexcept { return artistId_; }
const AlbumId& Track::albumId() const noexcept { return albumId_; }
std::chrono::milliseconds Track::duration() const noexcept { return duration_; }

void Track::setTitle(std::string title) { title_ = std::move(title); }
void Track::setDuration(std::chrono::milliseconds duration) noexcept { duration_ = duration; }

Artist::Artist(ArtistId id, std::string name) : id_(std::move(id)), name_(std::move(name)) {}

const ArtistId& Artist::id() const noexcept { return id_; }
const std::string& Artist::name() const noexcept { return name_; }
const std::vector<AlbumId>& Artist::albumIds() const noexcept { return albumIds_; }

void Artist::setName(std::string name) { name_ = std::move(name); }

void Artist::addAlbum(AlbumId albumId)
{
    if (std::find(albumIds_.begin(), albumIds_.end(), albumId) == albumIds_.end())
    {
        albumIds_.push_back(std::move(albumId));
    }
}

Album::Album(AlbumId id, std::string title, ArtistId artistId)
    : id_(std::move(id)), title_(std::move(title)), artistId_(std::move(artistId))
{
}

const AlbumId& Album::id() const noexcept { return id_; }
const std::string& Album::title() const noexcept { return title_; }
const ArtistId& Album::artistId() const noexcept { return artistId_; }
const std::vector<TrackId>& Album::trackIds() const noexcept { return trackIds_; }

void Album::setTitle(std::string title) { title_ = std::move(title); }

void Album::addTrack(TrackId trackId)
{
    if (std::find(trackIds_.begin(), trackIds_.end(), trackId) == trackIds_.end())
    {
        trackIds_.push_back(std::move(trackId));
    }
}

Playlist::Playlist(PlaylistId id, std::string name) : id_(std::move(id)), name_(std::move(name)) {}

const PlaylistId& Playlist::id() const noexcept { return id_; }
const std::string& Playlist::name() const noexcept { return name_; }
const std::vector<TrackId>& Playlist::trackIds() const noexcept { return trackIds_; }

void Playlist::setName(std::string name) { name_ = std::move(name); }
void Playlist::appendTrack(TrackId trackId) { trackIds_.push_back(std::move(trackId)); }
} // namespace music_surfer::core
