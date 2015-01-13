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

#include "Log.h"
#include <iostream>
#include <sstream>
#include <ctime>

std::ofstream Logger::WriteFileStream;
std::streambuf *Logger::CerrStream = NULL;
std::streambuf *Logger::CoutStream = NULL;

bool Logger::Initialize(std::string file)
{
    WriteFileStream.open(file.c_str());

    CerrStream = std::cerr.rdbuf(WriteFileStream.rdbuf());
    CoutStream = std::cout.rdbuf(WriteFileStream.rdbuf());

    return WriteFileStream.is_open();
}

void Logger::DeInitialize()
{
    if(WriteFileStream.is_open())
    {
        WriteFileStream.close();

    }

    std::cerr.rdbuf(CerrStream);
    std::cout.rdbuf(CoutStream);
}


void Logger::Write(Zone zone, std::string component, std::string message)
{
    std::string zoneStr;

    switch(zone)
    {
    case ZONE_INFO:
        zoneStr = "INFO";
        break;
    case ZONE_DEBUG:
        zoneStr = "DEBUG";
        break;
    case ZONE_WARNING:
        zoneStr = "WARNING";
        break;
    case ZONE_ERROR:
        zoneStr = "ERROR";
        break;
    }
    std::time_t rawtime = std::time(NULL);
    struct tm* timeinfo = std::localtime(&rawtime);

    static char timeStr[60];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

    std::stringstream ss;
    ss << "[" << timeStr << "] [" << zoneStr << "] [" << component << "] " << message << std::endl;
    std::cout << ss.str();
    std::cout.flush();
}
