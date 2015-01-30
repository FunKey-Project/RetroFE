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
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>

class Logger
{
public:
    enum Zone
    {
        ZONE_DEBUG,
        ZONE_INFO,
        ZONE_WARNING,
        ZONE_ERROR

    };
    static bool Initialize(std::string file);
    static void Write(Zone zone, std::string component, std::string message);
    static void DeInitialize();
private:

    static std::streambuf *CerrStream;
    static std::streambuf *CoutStream;
    static std::ofstream WriteFileStream;
};
