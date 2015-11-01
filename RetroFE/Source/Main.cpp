/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Database/Configuration.h"
#include "Utility/Log.h"
#include "Utility/Utils.h"
#include "RetroFE.h"
#include "Version.h"
#include <cstdlib>
#include <fstream>
#include <dirent.h>

static bool ImportConfiguration(Configuration *c);
static bool StartLogging();

int main(int argc, char **argv)
{
    Configuration::initialize();
    Configuration config;
    config.import("", "C:/Users/Don/Downloads/RetroFE-FTP/settings.conf");

    if(!StartLogging())
    {
        return -1;
    }

    RetroFE p(config);

    p.run();

    Logger::deInitialize();

    return 0;
}

bool StartLogging()
{
    std::string logFile = Utils::combinePath(Configuration::absolutePath, "log.txt");

    if(!Logger::initialize(logFile))
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not open \"" + logFile + "\" for writing");
        return false;
    }

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Version " + Version::getString() + " starting");

#ifdef WIN32
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Windows");
#else
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Linux");
#endif

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Absolute path: " + Configuration::absolutePath);

    return true;
}
