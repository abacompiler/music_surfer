#include "database/Database.hpp"

#include <utility>

namespace music_surfer::database
{
struct Database::Connection
{
    explicit Connection(std::filesystem::path filePath) : filePath(std::move(filePath)) {}
    std::filesystem::path filePath;
};

Database::Database() = default;
Database::~Database() = default;
Database::Database(Database&&) noexcept = default;
Database& Database::operator=(Database&&) noexcept = default;

bool Database::connect(const std::filesystem::path& databasePath)
{
    connection_ = std::make_unique<Connection>(databasePath);
    return true;
}

void Database::disconnect() noexcept { connection_.reset(); }

bool Database::isConnected() const noexcept { return static_cast<bool>(connection_); }

void Database::saveTrack(const core::Track& track)
{
    (void)track;
    // Schema and upsert operations will be wired to SQLite in a concrete integration pass.
}

std::optional<core::Track> Database::loadTrack(const core::TrackId& id) const
{
    (void)id;
    return std::nullopt;
}
} // namespace music_surfer::database
