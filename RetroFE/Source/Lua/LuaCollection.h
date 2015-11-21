#pragma once

#include "Lua.h"
#include "LuaEvent.h"
#include "../Collection/CollectionInfoBuilder.h"
#include "../Database/Configuration.h"

namespace LuaCollection
{
    void initialize(Configuration *c, CollectionInfoBuilder *b, LuaEvent *e);
    int load(lua_State *l);
    int destroy(lua_State *l);

    void loadCallback(void *context, CollectionInfo *info);
};