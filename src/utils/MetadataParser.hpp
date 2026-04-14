#pragma once

#include "services/Abstractions.hpp"

namespace music_surfer::utils
{
/**
 * @brief TagLib-based metadata adapter implementing service parsing abstraction.
 *
 * Lifecycle: stateless RAII object injected as `IMetadataReader`; avoids globals
 * so parser dependencies remain replaceable in tests and production.
 */
class MetadataParser final : public services::IMetadataReader
{
public:
    /** @brief Parse metadata for a single file into a track model when possible. */
    std::optional<core::Track> parseTrack(const std::filesystem::path& filePath) const override;
};
} // namespace music_surfer::utils
