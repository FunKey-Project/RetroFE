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

#include "Lua/Lua.h"
#include "Lua/LuaEvent.h"
#include "Database/Configuration.h"
#include "Graphics/Component/ComponentFactory.h"
#include <SDL2/SDL.h>
#include <vector>

class RetroFE
{
public:
    RetroFE(Configuration &c);
    virtual ~RetroFE();
    void run();
private:
    void initializeLua();
    void reloadLuaScripts();
    ComponentFactory factory_;
    Configuration &config_;
    Lua lua_;
    LuaEvent luaEvent_;
    static const luaL_Reg luaDisplayFuncs[];
    static const luaL_Reg luaLogFuncs[];
    static const luaL_Reg luaImageFuncs[];
};
