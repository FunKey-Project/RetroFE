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
#include "Graphics/Animate/AnimationManager.h"
#include "Collection/CollectionInfoBuilder.h"

#include "Database/DB.h"
#include "Database/Configuration.h"
#include "Database/MetadataDatabase.h"
#include "Graphics/Component/ComponentFactory.h"
#include "Lua/Lua.h"
#include "Lua/LuaEvent.h"
#include "Control/UserInput.h"
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
    AnimationManager animationManager_;
    static const luaL_Reg luaAnimateFuncs[];
    static const luaL_Reg luaCollectionFuncs[];
    static const luaL_Reg luaDisplayFuncs[];
    static const luaL_Reg luaImageFuncs[];
    static const luaL_Reg luaLogFuncs[];
    DB db_;
    MetadataDatabase *mdb_;
    CollectionInfoBuilder *cib_;
    UserInput input_;
};
