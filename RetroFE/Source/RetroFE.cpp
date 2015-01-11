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

#include "RetroFE.h"
#include "Database/CollectionDatabase.h"
#include "Database/Configuration.h"
#include "Collection/Item.h"
#include "Execute/Launcher.h"
#include "Utility/Log.h"
#include "Collection/MenuParser.h"
#include "SDL.h"
#include "Control/UserInput.h"
#include "Graphics/PageBuilder.h"
#include "Graphics/Page.h"
#include "Graphics/Component/ScrollingList.h"
#include "Video/VideoFactory.h"
#include <vector>
#include <string>
#include <sstream>
#ifdef WIN32
#include <Windows.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_thread.h>
#endif

RetroFE::RetroFE(Configuration &c)
    : Initialized(false)
    , InitializeThread(NULL)
    , Config(c)
    , Db(NULL)
    , CollectionDB(NULL)
    , Input(Config)
    , KeyInputDisable(0)
    , CurrentTime(0)
{
}

RetroFE::~RetroFE()
{
    DeInitialize();
}

CollectionDatabase *RetroFE::InitializeCollectionDatabase(DB &db, Configuration &config)
{
    CollectionDatabase *cdb = NULL;

    std::string dbFile = (Configuration::GetAbsolutePath() + "/cache.db");
    std::ifstream infile(dbFile.c_str());

    cdb = new CollectionDatabase(db, config);

    if(!cdb->Initialize())
    {
        delete cdb;
        cdb = NULL;
    }
    else if(!cdb->Import())
    {
        delete cdb;
        cdb = NULL;
    }

    return cdb;
}

void RetroFE::Render()
{
    SDL_LockMutex(SDL::GetMutex());
    SDL_SetRenderDrawColor(SDL::GetRenderer(), 0x0, 0x0, 0x00, 0xFF);
    SDL_RenderClear(SDL::GetRenderer());

    if(PageChain.size() > 0) 
    {
        Page *page = PageChain.back();

        if(page)
        {
            page->Draw();
        }
    }
    SDL_RenderPresent(SDL::GetRenderer());
    SDL_UnlockMutex(SDL::GetMutex());
}

int RetroFE::Initialize(void *context)
{
    int retVal = 0;
    RetroFE *instance = static_cast<RetroFE *>(context);

    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Initializing");
    bool videoEnable = true;
    int videoLoop = 0;

    if(!instance->Input.Initialize()) return -1;

    instance->Db = new DB(Configuration::GetAbsolutePath() + "/cache.db");

    if(!instance->Db->Initialize())
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not initialize database");
        return -1;
    }


    instance->CollectionDB = instance->InitializeCollectionDatabase(*instance->Db, instance->Config);

    if(!instance->CollectionDB)
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not initialize CollectionDB!");
        delete instance->Db;
        return -1;
    }


    instance->FC.Initialize();

    instance->Config.GetProperty("videoEnable", videoEnable);
    instance->Config.GetProperty("videoLoop", videoLoop);

    VideoFactory::SetEnabled(videoEnable);
    VideoFactory::SetNumLoops(videoLoop);

    instance->Initialized = true;
    return 0;
}

void RetroFE::LaunchEnter()
{
    if(PageChain.size() > 0)
    {
        Page *p = PageChain.back();
        p->LaunchEnter();
    }
    SDL_SetWindowGrab(SDL::GetWindow(), SDL_FALSE);

}

void RetroFE::LaunchExit()
{
    SDL_RestoreWindow(SDL::GetWindow());
    SDL_SetWindowGrab(SDL::GetWindow(), SDL_TRUE);

    if(PageChain.size() > 0)
    {
        Page *p = PageChain.back();
        p->LaunchExit();
    }

}

void RetroFE::FreeGraphicsMemory()
{
    if(PageChain.size() > 0)
    {
        Page *p = PageChain.back();
        p->FreeGraphicsMemory();
    }
    FC.DeInitialize();

    SDL::DeInitialize();
}

void RetroFE::AllocateGraphicsMemory()
{
    SDL::Initialize(Config);

    FC.Initialize();

    if(PageChain.size() > 0)
    {
        Page *p = PageChain.back();
        p->AllocateGraphicsMemory();
        p->Start();
    }
}

bool RetroFE::DeInitialize()
{
    bool retVal = true;
    FreeGraphicsMemory();

    bool videoEnable = true;

    while(PageChain.size() > 0)
    {
        Page *page = PageChain.back();
        delete page;
        PageChain.pop_back();
    }
    if(Db)
    {
        delete Db;
        Db = NULL;
    }

    if(CollectionDB)
    {
        delete CollectionDB;
        Db = NULL;
    }
    Initialized = false;
    //todo: handle video deallocation
    return retVal;
}

void RetroFE::Run()
{
    if(!SDL::Initialize(Config)) return;

    InitializeThread = SDL_CreateThread(Initialize, "RetroFEInit", (void *)this);

    if(!InitializeThread)
    {
        Logger::Write(Logger::ZONE_INFO, "RetroFE", "Could not initialize RetroFE");
        return;
    }

    int attractModeTime = 0;
    std::string firstCollection = "Main";
    bool running = true;
    Item *nextPageItem = NULL;
    bool adminMode = false;
    RETROFE_STATE state = RETROFE_NEW;

    Config.GetProperty("attractModeTime", attractModeTime);
    Config.GetProperty("firstCollection", firstCollection);

    Attract.SetIdleTime(static_cast<float>(attractModeTime));

    int initializeStatus = 0;

    // load the initial splash screen, unload it once it is complete
    Page * page = LoadSplashPage();
    bool splashMode = true;

    Launcher l(*this, Config);

    while (running)
    {
        float lastTime = 0;
        float deltaTime = 0;
        page = PageChain.back();

        if(!page)
        {
            Logger::Write(Logger::ZONE_WARNING, "RetroFE", "Could not load page");
            running = false;
            break;
        }
        // todo: This could be transformed to use the state design pattern.
        switch(state)
        {
        case RETROFE_IDLE:
            if(page && !splashMode)
            {
                state = ProcessUserInput(page);
            }

            if(Initialized && splashMode)
            {
                SDL_WaitThread(InitializeThread, &initializeStatus);
                state = RETROFE_BACK_WAIT;
                page->Stop();
            }

            break;

        case RETROFE_NEXT_PAGE_REQUEST:
            page->Stop();
            state = RETROFE_NEXT_PAGE_WAIT;
            break;

        case RETROFE_NEXT_PAGE_WAIT:
            if(page->IsHidden())
            {
                page = LoadPage(NextPageItem->GetName());
                state = RETROFE_NEW;
            }
            break;

        case RETROFE_LAUNCH_REQUEST:
            l.Run(page->GetCollectionName(), NextPageItem);
            state = RETROFE_IDLE;
            break;

        case RETROFE_BACK_REQUEST:
            page->Stop();
            state = RETROFE_BACK_WAIT;
            break;

        case RETROFE_BACK_WAIT:
            if(page->IsHidden())
            {
                PageChain.pop_back();
                delete page;

                page = (splashMode) ? LoadPage(firstCollection) : PageChain.back();
                splashMode = false;
                CurrentTime = (float)SDL_GetTicks() / 1000;

                page->AllocateGraphicsMemory();
                page->Start();
                state = RETROFE_NEW;
            }
            break;

        case RETROFE_NEW:
            if(page->IsIdle())
            {
                state = RETROFE_IDLE;
            }
            break;

        case RETROFE_QUIT_REQUEST:
            page->Stop();
            state = RETROFE_QUIT;
            break;

        case RETROFE_QUIT:
            if(page->IsHidden())
            {
                running = false;
            }

            break;
        }

        // the logic below could be done in a helper method
        if(running)
        {
            lastTime = CurrentTime;
            CurrentTime = (float)SDL_GetTicks() / 1000;

            if (CurrentTime < lastTime)
            {
                CurrentTime = lastTime;
            }

            deltaTime = CurrentTime - lastTime;
            double sleepTime = 1000.0/60.0 - deltaTime*1000;
            if(sleepTime > 0)
            {
                SDL_Delay(static_cast<unsigned int>(sleepTime));
            }

            if(page)
            {
                Attract.Update(deltaTime, *page);
                page->Update(deltaTime);
            }

            Render();
        }
    }

}


bool RetroFE::Back(bool &exit)
{
    bool canGoBack = false;

    bool exitOnBack = false;
    Config.GetProperty("exitOnFirstPageBack", exitOnBack);
    exit = false;

    if(PageChain.size() > 1)
    {
        Page *page = PageChain.back();
        page->Stop();
        canGoBack = true;
    }
    else if(PageChain.size() == 1 && exitOnBack)
    {
        Page *page = PageChain.back();
        page->Stop();
        exit = true;
        canGoBack = true;
    }

    return canGoBack;
}


RetroFE::RETROFE_STATE RetroFE::ProcessUserInput(Page *page)
{
    SDL_Event e;
    bool exit = false;
    RETROFE_STATE state = RETROFE_IDLE;

    if (SDL_PollEvent(&e) == 0) return state;

    if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
    {
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        Attract.Reset();

        if (keys[Input.GetScancode(UserInput::KeyCodePreviousItem)])
        {
            page->SetScrolling(Page::ScrollDirectionBack);
        }
        if (keys[Input.GetScancode(UserInput::KeyCodeNextItem)])
        {
            page->SetScrolling(Page::ScrollDirectionForward);
        }
        if (keys[Input.GetScancode(UserInput::KeyCodePageUp)])
        {
            page->PageScroll(Page::ScrollDirectionBack);
        }
        if (keys[Input.GetScancode(UserInput::KeyCodePageDown)])
        {
            page->PageScroll(Page::ScrollDirectionForward);
        }
        if (keys[Input.GetScancode(UserInput::KeyCodeAdminMode)])
        {
            //todo: add admin mode support
        }
        if (keys[Input.GetScancode(UserInput::KeyCodeSelect)])
        {
            NextPageItem = page->GetSelectedItem();

            if(NextPageItem)
            {
                state = (NextPageItem->IsLeaf()) ? RETROFE_LAUNCH_REQUEST : RETROFE_NEXT_PAGE_REQUEST;
            }
        }

        if (keys[Input.GetScancode(UserInput::KeyCodeBack)])
        {
            if(Back(exit))
            {
                state = (exit) ? RETROFE_QUIT_REQUEST : RETROFE_BACK_REQUEST;
            }
        }

        if (keys[Input.GetScancode(UserInput::KeyCodeQuit)])
        {
            state = RETROFE_QUIT_REQUEST;
        }

        if(!keys[Input.GetScancode(UserInput::KeyCodePreviousItem)] &&
                !keys[Input.GetScancode(UserInput::KeyCodeNextItem)] &&
                !keys[Input.GetScancode(UserInput::KeyCodePageUp)] &&
                !keys[Input.GetScancode(UserInput::KeyCodePageDown)])
        {
            page->SetScrolling(Page::ScrollDirectionIdle);
        }
    }

    return state;
}


void RetroFE::WaitToInitialize()
{
    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Loading splash screen");

    PageBuilder pb("splash", "", Config, &FC);
    Page * page = pb.BuildPage();

    while(!Initialized)
    {
    SDL_SetRenderDrawColor(SDL::GetRenderer(), 0x0, 0x0, 0x00, 0xFF);
    SDL_RenderClear(SDL::GetRenderer());
//    image->Draw();
    //todo: decouple page from a collection
    page->Draw();
    SDL_RenderPresent(SDL::GetRenderer());

        SDL_Delay(2000);
    }

    int status = 0;
    delete page;    
    SDL_WaitThread(InitializeThread, &status);
}


Page *RetroFE::LoadPage(std::string collectionName)
{
    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Creating page for collection " + collectionName);

    Page *page = NULL;

    std::vector<Item *> *collection = GetCollection(collectionName);
    std::string layoutName = GetLayout(collectionName);

    if(PageChain.size() > 0)
    {
        Page *oldPage = PageChain.back();
        oldPage->FreeGraphicsMemory();
    }

    PageBuilder pb(layoutName, collectionName, Config, &FC);
    page = pb.BuildPage();

    if(!page)
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not create page for " + collectionName);
    }
    else
    {
        page->SetItems(collection);
        page->Start();

        PageChain.push_back(page);
    }

    return page;
}
Page *RetroFE::LoadSplashPage()
{
    PageBuilder pb("splash", "", Config, &FC);
    std::vector<Item *> *coll = new std::vector<Item *>();
    Page * page = pb.BuildPage();
    page->SetItems(coll);
    page->Start();
    PageChain.push_back(page);

    return page;
}


std::vector<Item *> *RetroFE::GetCollection(std::string collectionName)
{
    // the page will deallocate this once its done
    std::vector<Item *> *collection = new std::vector<Item *>(); 
    MenuParser mp;

    mp.GetMenuItems(CollectionDB, collectionName, *collection);
    CollectionDB->GetCollection(collectionName, *collection);

    if(collection->size() == 0)
    {
        Logger::Write(Logger::ZONE_WARNING, "RetroFE", "No list items found for collection " + collectionName);
    }

    return collection;
}

std::string RetroFE::GetLayout(std::string collectionName)
{
    std::string layoutKeyName = "collections." + collectionName + ".layout";
    std::string layoutName = "Default 16x9";

    if(!Config.GetProperty(layoutKeyName, layoutName))
    {
        Config.GetProperty("layout", layoutName);
    }

    return layoutName;
}