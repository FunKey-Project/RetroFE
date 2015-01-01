/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Collection/Item.h"
#include "Control/UserInput.h"
#include "Graphics/FontCache.h"
#include "Video/IVideo.h"
#include <SDL2/SDL.h>
#include <list>

class CollectionDatabase;
class Configuration;
class Page;

class RetroFE
{
public:
    RetroFE(CollectionDatabase *db, Configuration *c);
    virtual ~RetroFE();
    bool Initialize();
    bool DeInitialize();
    void Run();
    Configuration *GetConfiguration();
    void FreeGraphicsMemory();
    void AllocateGraphicsMemory();
    void LaunchEnter();
    void LaunchExit();
private:
    enum RETROFE_STATE
    {
        RETROFE_IDLE,
        RETROFE_NEXT_PAGE_REQUEST,
        RETROFE_NEXT_PAGE_WAIT,
        RETROFE_LAUNCH_REQUEST,
        RETROFE_BACK_REQUEST,
        RETROFE_BACK_WAIT,
        RETROFE_NEW,
        RETROFE_QUIT_REQUEST,
        RETROFE_QUIT,
    };

    void Render();
    bool ItemSelected();
    bool Back(bool &exit);
    void Quit();
    Page *LoadPage(std::string collectionName);
    RETROFE_STATE ProcessUserInput();
    void Update(float dt, bool scrollActive);


    Configuration *Config;
    CollectionDatabase *CollectionDB;
    UserInput Input;
    std::list<Page *> PageChain;
    float KeyInputDisable;
    float InactiveKeyTime;
    bool AttractMode;
    float CurrentTime;
    Item *NextPageItem;
    FontCache FC;
    IVideo *VideoInst;

};
