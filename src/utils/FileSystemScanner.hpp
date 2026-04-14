#pragma once

#include "services/Abstractions.hpp"

#include <unordered_set>

namespace music_surfer::utils
{
/**
 * @brief File-system adapter that discovers audio files for library ingestion.
 *
 * Lifecycle: lightweight RAII object with no shared global cache; inject through
 * `IFileScanner` so services remain decoupled from filesystem implementation.
 */
class FileSystemScanner final : public services::IFileScanner
{
public:
    /** @brief Construct scanner with a default set of supported extensions. */
    FileSystemScanner();

    /** @brief Enumerate matching audio files under a root directory. */
    std::vector<std::filesystem::path> scanAudioFiles(const std::filesystem::path& root) const override;

private:
    std::unordered_set<std::string> extensions_;
};
} // namespace music_surfer::utils
