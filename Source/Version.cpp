/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Version.h"
#include <sstream>

#ifndef RETROFE_VERSION_MAJOR
#define RETROFE_VERSION_MAJOR 0
#endif

#ifndef RETROFE_VERSION_MINOR
#define RETROFE_VERSION_MINOR 0
#endif

#ifndef RETROFE_VERSION_BUILD
#define RETROFE_VERSION_BUILD 0
#endif

#ifndef RETROFE_VERSION_PROD
#define RETROFE_VERSION_BETA
#endif

std::string Version::GetString()
{
   std::stringstream version;
   version << RETROFE_VERSION_MAJOR;
   version << ".";
   version << RETROFE_VERSION_MINOR;
   version << ".";
   version << RETROFE_VERSION_BUILD;

#ifdef RETROFE_VERSION_BETA
   version << "-beta";
#endif

   return version.str();
}
