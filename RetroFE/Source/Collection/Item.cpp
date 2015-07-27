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

#include "Item.h"
#include "../Utility/Utils.h"
#include <sstream>
#include <algorithm>

Item::Item()
    : collectionInfo(NULL)
    , leaf(true)
{
}

Item::~Item()
{
}

std::string Item::filename()
{
    return Utils::getFileName(filepath);
}



std::string Item::lowercaseTitle()
{
    std::string lcstr = title;
    std::transform(lcstr.begin(), lcstr.end(), lcstr.begin(), ::tolower);
    return lcstr;
}

std::string Item::lowercaseFullTitle()
{
    std::string lcstr = fullTitle;
    std::transform(lcstr.begin(), lcstr.end(), lcstr.begin(), ::tolower);
    return lcstr;
}

