/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Log.h"
#include <iostream>
#include <sstream>
#include <ctime>

std::ofstream Logger::WriteFileStream;
std::streambuf *Logger::CerrStream = NULL;
std::streambuf *Logger::CoutStream = NULL;

bool Logger::StartLogFile(std::string file)
{
   WriteFileStream.open(file.c_str());

   CerrStream = std::cerr.rdbuf(WriteFileStream.rdbuf());
   CoutStream = std::cout.rdbuf(WriteFileStream.rdbuf());

   return WriteFileStream.is_open();
}

void Logger::CloseLogFile()
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
}
