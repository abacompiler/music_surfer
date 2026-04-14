#include "services/AudioPlayer.hpp"

namespace music_surfer::services
{
void AudioPlayer::load(const core::Track& track)
{
    loadedTrack_.reset();
    loadedTrack_.emplace(track);
    state_ = State::Stopped;
    position_ = std::chrono::milliseconds::zero();
}

void AudioPlayer::play()
{
    if (loadedTrack_)
    {
        state_ = State::Playing;
    }
}

void AudioPlayer::pause()
{
    if (state_ == State::Playing)
    {
        state_ = State::Paused;
    }
}

void AudioPlayer::stop()
{
    state_ = State::Stopped;
    position_ = std::chrono::milliseconds::zero();
}

void AudioPlayer::seek(std::chrono::milliseconds position)
{
    if (!loadedTrack_)
    {
        return;
    }

    if (position < std::chrono::milliseconds::zero())
    {
        position_ = std::chrono::milliseconds::zero();
        return;
    }

    const auto maxDuration = loadedTrack_->duration();
    position_ = position > maxDuration ? maxDuration : position;
}

std::optional<core::Track> AudioPlayer::currentTrack() const { return loadedTrack_; }

AudioPlayer::State AudioPlayer::state() const noexcept { return state_; }

std::chrono::milliseconds AudioPlayer::position() const noexcept { return position_; }
} // namespace music_surfer::services
