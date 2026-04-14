#pragma once

#include "core/Track.hpp"

#include <chrono>
#include <optional>

namespace music_surfer::services
{
/**
 * @brief Playback service with basic transport controls independent of backend.
 */
class AudioPlayer
{
public:
    enum class State
    {
        Stopped,
        Playing,
        Paused,
    };

    /** @brief Load a track for future playback without starting transport. */
    void load(const core::Track& track);
    /** @brief Start playback of the loaded track. */
    void play();
    /** @brief Pause playback while preserving current position. */
    void pause();
    /** @brief Stop playback and reset transport position. */
    void stop();
    /** @brief Seek to an absolute position (clamped to track duration). */
    void seek(std::chrono::milliseconds position);

    /** @brief Return currently loaded track, if any. */
    std::optional<core::Track> currentTrack() const;
    /** @brief Return current transport state. */
    State state() const noexcept;
    /** @brief Return current playback position. */
    std::chrono::milliseconds position() const noexcept;

private:
    std::optional<core::Track> loadedTrack_;
    State state_{State::Stopped};
    std::chrono::milliseconds position_{0};
};
} // namespace music_surfer::services
