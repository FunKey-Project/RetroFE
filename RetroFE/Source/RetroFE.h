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
#include <SDL2/SDL.h>
#include <list>
#include <vector>

class CollectionInfo;
class Configuration;
class Page;

class RetroFE
{
public:
    RetroFE(Configuration &c);
    virtual ~RetroFE();
    bool DeInitialize();
    void Run();
    void FreeGraphicsMemory();
    void AllocateGraphicsMemory();
    void LaunchEnter();
    void LaunchExit();
private:
    bool Initialized;
    SDL_Thread *InitializeThread;
    static int Initialize(void *context);

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

    void Render();
    bool Back(bool &exit);
    void Quit();
    void WaitToInitialize();
    Page *LoadPage();
    Page *LoadSplashPage();
    RETROFE_STATE ProcessUserInput(Page *page);
    void Update(float dt, bool scrollActive);
    std::string GetLayout(std::string collectionName);
    CollectionInfo *GetCollection(std::string collectionName);
    Configuration &Config;
    DB *Db;
    MetadataDatabase *MetaDb;
    UserInput Input;
    Page *CurrentPage;
    float KeyInputDisable;
    float CurrentTime;
    Item *NextPageItem;
    FontCache FC;
    AttractMode Attract;

};
