#include "database/Database.hpp"

#include <iostream>

int main()
{
    music_surfer::database::Database database("music_surfer.db");
    if (!database.isConnected())
    {
        std::cerr << "Failed to initialize database at startup" << std::endl;
    }

    std::cout << "MusicSurfer starter app" << std::endl;
    return 0;
}
