#pragma once

#include "core/Identifiers.hpp"

#include <string>
#include <vector>

namespace music_surfer::core
{
/**
 * @brief Artist aggregate root with immutable identity and album references.
 *
 * Lifecycle: non-owning references are expressed via IDs; object lifetime is
 * managed by service-layer smart pointers.
 */
class Artist
{
public:
    /** @brief Construct an artist with immutable identity and display name. */
    Artist(ArtistId id, std::string name);

    /** @brief Return immutable artist identifier. */
    const ArtistId& id() const noexcept;
    /** @brief Return artist display name. */
    const std::string& name() const noexcept;
    /** @brief Return referenced album IDs associated with the artist. */
    const std::vector<AlbumId>& albumIds() const noexcept;

    /** @brief Update mutable artist display name. */
    void setName(std::string name);
    /** @brief Associate an album ID with this artist if it is not already present. */
    void addAlbum(AlbumId albumId);

private:
    const ArtistId id_;
    std::string name_;
    std::vector<AlbumId> albumIds_;
};
} // namespace music_surfer::core
