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


#include "Menu.h"
#include "../Collection/Item.h"
#include <iostream>


Menu::Menu( Configuration &c )
    : config_( c )
{
    page_ = nullptr;
}


void Menu::handleEntry( Item *item )
{
    std::cout << "Handling " + item->ctrlType + "." << std::endl;
    return;
}


void Menu::setPage( Page *page )
{
    page_ = page;
}


void Menu::clearPage( )
{
    page_ = nullptr;
}
