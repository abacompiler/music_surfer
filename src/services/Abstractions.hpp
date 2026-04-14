#pragma once

#include "core/Track.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace music_surfer::services
{
/**
 * @brief Storage abstraction for persisting and loading track models.
 *
 * Lifecycle: implemented by infrastructure adapters (e.g., SQLite wrapper),
 * injected into services via smart pointers, and never stored globally.
 */
class ITrackRepository
{
public:
    virtual ~ITrackRepository() = default;

    /** @brief Persist or update a track record. */
    virtual void saveTrack(const core::Track& track) = 0;
    /** @brief Load a track by ID if present. */
    virtual std::optional<core::Track> loadTrack(const core::TrackId& id) const = 0;
};

/**
 * @brief Abstraction for scanning the file system for candidate audio files.
 *
 * Lifecycle: stateless or RAII-managed implementations are owned by callers via
 * smart pointers to keep service dependencies inverted.
 */
class IFileScanner
{
public:
    virtual ~IFileScanner() = default;

    /** @brief Enumerate audio files rooted at a library path. */
    virtual std::vector<std::filesystem::path> scanAudioFiles(const std::filesystem::path& root) const = 0;
};

/**
 * @brief Abstraction for extracting track metadata from an audio file.
 *
 * Lifecycle: implemented by infrastructure wrappers (e.g., TagLib), injected
 * as an interface dependency with no global singletons.
 */
class IMetadataReader
{
public:
    virtual ~IMetadataReader() = default;

    /** @brief Parse metadata from a file and return a track when successful. */
    virtual std::optional<core::Track> parseTrack(const std::filesystem::path& filePath) const = 0;
};
} // namespace music_surfer::services
