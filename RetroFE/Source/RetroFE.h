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

#include "Collection/Item.h"
#include "Control/UserInput.h"
#include "Database/DB.h"
#include "Database/MetadataDatabase.h"
#include "Execute/AttractMode.h"
#include "Graphics/FontCache.h"
#include "Video/IVideo.h"
#include "Video/VideoFactory.h"
#include <SDL2/SDL.h>
#include <list>
#include <vector>
#include <map>

class CollectionInfo;
class Configuration;
class Page;

class RetroFE
{
public:
    RetroFE(Configuration &c);
    virtual ~RetroFE();
    bool deInitialize();
    void run();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void launchEnter();
    void launchExit();
private:
    volatile bool initialized;
    volatile bool initializeError;
    SDL_Thread *initializeThread;
    static int initialize(void *context);

    enum RETROFE_STATE
    {
        RETROFE_IDLE,
        RETROFE_NEXT_PAGE_REQUEST,
        RETROFE_LAUNCH_REQUEST,
        RETROFE_BACK_REQUEST,
        RETROFE_NEW,
        RETROFE_QUIT_REQUEST,
        RETROFE_QUIT,
    };

    void render();
    bool back(bool &exit);
    void quit();
    Page *loadPage();
    Page *loadSplashPage();
    RETROFE_STATE processUserInput(Page *page);
    void update(float dt, bool scrollActive);
    std::string getLayout(std::string collectionName);
    CollectionInfo *getCollection(std::string collectionName);
    Configuration &config_;
    DB *db_;
    MetadataDatabase *metadb_;
    UserInput input_;
    Page *currentPage_;
    float keyInputDisable_;
    float currentTime_;
    Item *nextPageItem_;
    FontCache fontcache_;
    AttractMode attract_;
    std::map<std::string, unsigned int> lastMenuOffsets_;

};
