/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
