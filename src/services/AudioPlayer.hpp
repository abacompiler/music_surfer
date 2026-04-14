#pragma once

#include "core/Track.hpp"

#include <optional>

namespace music_surfer::services
{
/**
 * @brief Service interface for playback control independent of audio backend.
 *
 * Lifecycle: backend implementations hold native resources via RAII and are
 * consumed as interface pointers to avoid concrete engine coupling.
 */
class AudioPlayer
{
public:
    virtual ~AudioPlayer() = default;

    /** @brief Load a track for future playback without starting transport. */
    virtual void load(const core::Track& track) = 0;
    /** @brief Start playback of the loaded track. */
    virtual void play() = 0;
    /** @brief Pause playback while preserving current position. */
    virtual void pause() = 0;
    /** @brief Stop playback and reset transport state. */
    virtual void stop() = 0;

    /** @brief Return currently loaded track, if any. */
    virtual std::optional<core::Track> currentTrack() const = 0;
};
} // namespace music_surfer::services
