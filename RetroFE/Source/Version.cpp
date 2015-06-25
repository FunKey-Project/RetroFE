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

std::string Version::getString()
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
