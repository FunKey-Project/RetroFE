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
#include "Collection/CollectionInfoBuilder.h"
#include "Collection/CollectionInfo.h"
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
    , Input(Config)
    , CurrentPage(NULL)
    , KeyInputDisable(0)
    , CurrentTime(0)
{
}

RetroFE::~RetroFE()
{
    DeInitialize();
}

void RetroFE::Render()
{
    SDL_LockMutex(SDL::GetMutex());
    SDL_SetRenderDrawColor(SDL::GetRenderer(), 0x0, 0x0, 0x00, 0xFF);
    SDL_RenderClear(SDL::GetRenderer());

    if(CurrentPage)
    {
        CurrentPage->Draw();
    }

    SDL_RenderPresent(SDL::GetRenderer());
    SDL_UnlockMutex(SDL::GetMutex());
}

int RetroFE::Initialize(void *context)
{
    RetroFE *instance = static_cast<RetroFE *>(context);

    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Initializing");

    if(!instance->Input.Initialize()) return -1;

    instance->Db = new DB(Configuration::GetAbsolutePath() + "/meta.db");

    if(!instance->Db->Initialize())
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not initialize database");
        return -1;
    }

    instance->MetaDb = new MetadataDatabase(*(instance->Db), instance->Config);

    if(!instance->MetaDb->Initialize())
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not initialize meta database");
        return -1;
    }

    instance->Initialized = true;
    return 0;
}

void RetroFE::LaunchEnter()
{
    if(CurrentPage)
    {
        CurrentPage->LaunchEnter();
    }

    SDL_SetWindowGrab(SDL::GetWindow(), SDL_FALSE);

}

void RetroFE::LaunchExit()
{
    SDL_RestoreWindow(SDL::GetWindow());
    SDL_SetWindowGrab(SDL::GetWindow(), SDL_TRUE);

    if(CurrentPage)
    {
        CurrentPage->LaunchExit();
    }

}

void RetroFE::FreeGraphicsMemory()
{
    if(CurrentPage)
    {
        CurrentPage->FreeGraphicsMemory();
    }
    FC.DeInitialize();

    SDL::DeInitialize();
}

void RetroFE::AllocateGraphicsMemory()
{
    SDL::Initialize(Config);

    FC.Initialize();

    if(CurrentPage)
    {
        CurrentPage->AllocateGraphicsMemory();
        CurrentPage->Start();
    }
}

bool RetroFE::DeInitialize()
{
    bool retVal = true;
    FreeGraphicsMemory();

    if(CurrentPage)
    {
        delete CurrentPage;
        CurrentPage = NULL;
    }
    if(MetaDb)
    {
        delete MetaDb;
        MetaDb = NULL;
    }

    if(Db)
    {
        delete Db;
        Db = NULL;
    }

    Initialized = false;
    //todo: handle video deallocation

    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Exiting");

    return retVal;
}

void RetroFE::Run()
{
    if(!SDL::Initialize(Config)) return;

    FC.Initialize();

    bool videoEnable = true;
    int videoLoop = 0;
    Config.GetProperty("videoEnable", videoEnable);
    Config.GetProperty("videoLoop", videoLoop);

    VideoFactory::SetEnabled(videoEnable);
    VideoFactory::SetNumLoops(videoLoop);
    VideoFactory::CreateVideo(); // pre-initialize the gstreamer engine


    InitializeThread = SDL_CreateThread(Initialize, "RetroFEInit", (void *)this);

    if(!InitializeThread)
    {
        Logger::Write(Logger::ZONE_INFO, "RetroFE", "Could not initialize RetroFE");
        return;
    }

    int attractModeTime = 0;
    std::string firstCollection = "Main";
    bool running = true;
    RETROFE_STATE state = RETROFE_NEW;

    Config.GetProperty("attractModeTime", attractModeTime);
    Config.GetProperty("firstCollection", firstCollection);

    Attract.SetIdleTime(static_cast<float>(attractModeTime));

    int initializeStatus = 0;

    // load the initial splash screen, unload it once it is complete
    CurrentPage = LoadSplashPage();
    bool splashMode = true;

    Launcher l(*this, Config);

    while (running)
    {
        float lastTime = 0;
        float deltaTime = 0;

        if(!CurrentPage)
        {
            Logger::Write(Logger::ZONE_WARNING, "RetroFE", "Could not load page");
            running = false;
            break;
        }
        // todo: This could be transformed to use the state design pattern.
        switch(state)
        {
        case RETROFE_IDLE:
            if(CurrentPage && !splashMode)
            {
                state = ProcessUserInput(CurrentPage);
            }
            else
            {
                // read and discard SDL input to prevent windows from balking at us
                SDL_Event e;
                (void)SDL_PollEvent(&e);
            }

            if(Initialized && splashMode)
            {
                SDL_WaitThread(InitializeThread, &initializeStatus);

                // delete the splash screen and use the standard menu
                delete CurrentPage;
                CurrentPage = LoadPage();
                splashMode = false;
                if(CurrentPage)
                {
                    std::string firstCollection = "Main";
                    Config.GetProperty("firstCollection", firstCollection);

                    CurrentPage->Start();
                    Config.SetCurrentCollection(firstCollection);
                    CollectionInfo *info = GetCollection(firstCollection);
                    MenuParser mp;
                    mp.GetMenuItems(info);
                    CurrentPage->PushCollection(info);
                }
                else
                {
                    state = RETROFE_QUIT_REQUEST;
                }

            }

            break;

        case RETROFE_NEXT_PAGE_REQUEST:
            if(CurrentPage->IsIdle())
            {
                state = RETROFE_NEW;
            }
            break;

        case RETROFE_LAUNCH_REQUEST:
            NextPageItem = CurrentPage->GetSelectedItem();
            l.Run(CurrentPage->GetCollectionName(), NextPageItem);
            state = RETROFE_IDLE;
            break;

        case RETROFE_BACK_REQUEST:
            CurrentPage->PopCollection();
            Config.SetCurrentCollection(CurrentPage->GetCollectionName());

            state = RETROFE_NEW;

            break;

        case RETROFE_NEW:
            if(CurrentPage->IsIdle())
            {
                state = RETROFE_IDLE;
            }
            break;

        case RETROFE_QUIT_REQUEST:
            CurrentPage->Stop();
            state = RETROFE_QUIT;
            break;

        case RETROFE_QUIT:
            if(CurrentPage->IsHidden())
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

            if(CurrentPage)
            {
                Attract.Update(deltaTime, *CurrentPage);
                CurrentPage->Update(deltaTime);
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

    if(CurrentPage->GetMenuDepth() == 0)
    {
        exit = exitOnBack;
    }
    else
    {
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

	if(page->IsHorizontalScroll())
        {
            if (keys[Input.GetScancode(UserInput::KeyCodeLeft)])
            {
                page->SetScrolling(Page::ScrollDirectionBack);
            }
            if (keys[Input.GetScancode(UserInput::KeyCodeRight)])
            {
               page->SetScrolling(Page::ScrollDirectionForward);
            } 
            if (keys[Input.GetScancode(UserInput::KeyCodeUp)])
            {
                page->SetScrolling(Page::ScrollDirectionBack);
            }
            if (keys[Input.GetScancode(UserInput::KeyCodeDown)])
            {
                 page->SetScrolling(Page::ScrollDirectionForward);
            }
        }
        else
        { 
            if (keys[Input.GetScancode(UserInput::KeyCodePageUp)])
            {
                page->PageScroll(Page::ScrollDirectionBack);
            }
            if (keys[Input.GetScancode(UserInput::KeyCodePageDown)])
            {
                page->PageScroll(Page::ScrollDirectionForward);
            }
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
                if(NextPageItem->IsLeaf())
                {
                    state = RETROFE_LAUNCH_REQUEST;
                }
                else
                {
                    Config.SetCurrentCollection(NextPageItem->GetName());
                    CollectionInfo *info = GetCollection(NextPageItem->GetName());

                    page->PushCollection(info);
                    state = RETROFE_NEXT_PAGE_REQUEST;
                }
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

        if(!keys[Input.GetScancode(UserInput::KeyCodeUp)] &&
                !keys[Input.GetScancode(UserInput::KeyCodeLeft)] &&
                !keys[Input.GetScancode(UserInput::KeyCodeDown)] &&
                !keys[Input.GetScancode(UserInput::KeyCodeRight)] &&
                !keys[Input.GetScancode(UserInput::KeyCodePageUp)] &&
                !keys[Input.GetScancode(UserInput::KeyCodePageDown)])
        {
            page->SetScrolling(Page::ScrollDirectionIdle);
        }
    }

    return state;
}

Page *RetroFE::LoadPage()
{
    std::string layoutName;

    Config.GetProperty("layout", layoutName);

    PageBuilder pb(layoutName, "layout", Config, &FC);
    Page *page = pb.BuildPage();

    if(!page)
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not create page");
    }
    else
    {
        page->Start();
    }

    return page;
}

Page *RetroFE::LoadSplashPage()
{
    std::string layoutName;
    Config.GetProperty("layout", layoutName);

    PageBuilder pb(layoutName, "splash", Config, &FC);
    Page * page = pb.BuildPage();
    page->Start();

    return page;
}


CollectionInfo *RetroFE::GetCollection(std::string collectionName)
{
    // the page will deallocate this once its done

    CollectionInfoBuilder cib(Config, *MetaDb);
    CollectionInfo *collection = cib.BuildCollection(collectionName);

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
