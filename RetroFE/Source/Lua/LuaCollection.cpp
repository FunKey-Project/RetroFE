#include "LuaCollection.h"
#include "../Collection/CollectionInfo.h"
#include <string>
#include "../Collection/Item.h"

static Configuration *config;
static CollectionInfoBuilder *cib;
static LuaEvent *events;
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

int LuaCollection::getSize(lua_State *l)
{
    CollectionInfo *i = (CollectionInfo *)lua_touserdata(l, 1);

    lua_pushinteger(l, (int)i->items.size());

    return 1;
}

int LuaCollection::getName(lua_State *l)
{
    CollectionInfo *i = (CollectionInfo *)lua_touserdata(l, 1);

    lua_pushstring(l, i->name.c_str());

    return 1;
}

int LuaCollection::getItemAt(lua_State *l)
{
    CollectionInfo *i = (CollectionInfo *)lua_touserdata(l, 1);
    int index = (int)luaL_checkinteger(l, 2);
    Item *item = i->items.at(index);

    std::string name;
    std::string filepath;
    std::string title;
    std::string fullTitle;
    std::string year;
    std::string manufacturer;
    std::string genre;
    std::string cloneof;
    std::string numberPlayers;
    std::string numberButtons;

    lua_createtable(l, 0, 4);
    lua_pushstring(l, "name");
    lua_pushstring(l, item->name.c_str());
    lua_settable(l, -3);

    lua_pushstring(l, "title");
    lua_pushstring(l, item->title.c_str());
    lua_settable(l, -3);

    lua_pushstring(l, "filepath");
    lua_pushstring(l, item->filepath.c_str());
    lua_settable(l, -3);

    return 1;
}


int LuaCollection::destroy(lua_State *l)
{
    CollectionInfo *i = (CollectionInfo *)lua_touserdata(l, 1);
    cib->destroyCollection(i);
    
    return 0; 
}


CollectionInfo *loadingCollection = NULL;
void LuaCollection::loadCallback(void *context, CollectionInfo *info)
{
    loadingCollection = info;
}

void LuaCollection::update(lua_State *l) {
    if(loadingCollection) {
    events->trigger(l, "onCollectionLoaded", (void *)loadingCollection);
    loadingCollection = NULL;

    }
}

