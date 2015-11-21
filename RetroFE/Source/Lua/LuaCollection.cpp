#include "LuaCollection.h"
#include "../Collection/CollectionInfo.h"
#include <string>
#include "../Collection/Item.h"

Configuration *config;
CollectionInfoBuilder *cib;
LuaEvent *events;
void LuaCollection::initialize(Configuration *c, CollectionInfoBuilder *b,  LuaEvent *e)
{
    config = c;
    cib = b;
    events = e;
}

int LuaCollection::load(lua_State *l)
{
    std::string collection = luaL_checkstring(l, 1);
    cib->buildCollection(collection, loadCallback, (void *)l);
    
    return 0; 
}

void LuaCollection::loadCallback(void *context)
{
    lua_State *l = (lua_State *)context;
    events->trigger(l, "onCollectionLoaded");

}
