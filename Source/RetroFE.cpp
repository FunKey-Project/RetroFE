/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
#endif


Page *page = NULL;

RetroFE::RetroFE(CollectionDatabase &db, Configuration &c)
    : Config(c)
    , CollectionDB(db)
    , Input(Config)
    , KeyInputDisable(0)
    , InactiveKeyTime(0)
    , AttractMode(false)
    , CurrentTime(0)
    , VideoInst(NULL)
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

    Page *page = PageChain.back();

    if(page)
    {
        page->Draw();
    }

    SDL_RenderPresent(SDL::GetRenderer());
    SDL_UnlockMutex(SDL::GetMutex());
}

bool RetroFE::Initialize()
{
    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Initializing");

    if(!Input.Initialize()) return false;
    if(!SDL::Initialize(Config)) return false;
    FC.Initialize();

    bool videoEnable = true;
    int videoLoop = 0;

    Config.GetProperty("videoEnable", videoEnable);
    Config.GetProperty("videoLoop", videoLoop);

    VideoFactory::SetEnabled(videoEnable);
    VideoFactory::SetNumLoops(videoLoop);
    VideoFactory vf;
    VideoInst = vf.CreateVideo();

    return true;
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

    if(VideoInst)
    {
        delete VideoInst;
        VideoInst = NULL;
    }

    //todo: handle video deallocation
    return retVal;
}

void RetroFE::Run()
{
    int attractModeTime = 0;
    std::string firstCollection = "Main";

    Config.GetProperty("attractModeTime", attractModeTime);
    Config.GetProperty("firstCollection", firstCollection);

    bool running = true;
    Item *nextPageItem = NULL;
    bool adminMode = false;
    float attractModeRandomTime = 0;
    bool selectActive = false;

    //todo: break up into helper methods
    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Loading first page");

    page = LoadPage(firstCollection);

    float frameCount = 0;
    float fpsStartTime = 0;
    RETROFE_STATE state = RETROFE_IDLE;

    while (running)
    {
        float lastTime = 0;
        float deltaTime = 0;
        page = PageChain.back();
        Launcher l(*this, Config);

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
            state = ProcessUserInput();
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

                page = PageChain.back();
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

            ++frameCount;

            if(CurrentTime - fpsStartTime > 1.0)
            {
                // don't print the first framerate, it's likely inaccurate
                bool logFps = false;
                Config.GetProperty("debug.logfps", logFps);

                if(fpsStartTime != 0 && logFps)
                {
                    std::stringstream fpsstream;
                    fpsstream << frameCount/(CurrentTime - fpsStartTime) << " FPS";
                    Logger::Write(Logger::ZONE_DEBUG, "RetroFE", fpsstream.str());
                }

                fpsStartTime = CurrentTime;
                frameCount = 0;
            }

            InactiveKeyTime += deltaTime;

            if(!AttractMode && InactiveKeyTime > attractModeTime && attractModeTime > 0)
            {
                AttractMode = true;
                InactiveKeyTime = 0;
                attractModeRandomTime = ((float)((1000+rand()) % 5000)) / 1000;
            }

            if(AttractMode)
            {
                page->SetScrolling(Page::ScrollDirectionForward);

                if(InactiveKeyTime > attractModeRandomTime)
                {
                    InactiveKeyTime = 0;
                    AttractMode = false;
                    page->SetScrolling(Page::ScrollDirectionIdle);
                }
            }

            page->Update(deltaTime);
            Render();
        }
    }
}

bool RetroFE::ItemSelected()
{
    Item *item = page->GetSelectedItem();

    if(!item) return false;

    if(item->IsLeaf())
    {
        Launcher l(*this, Config);

        l.Run(page->GetCollectionName(), item);
    }
    else
    {
        NextPageItem = item;
        LoadPage(page->GetCollectionName());
        page->Stop();
    }

    return true;
}

bool RetroFE::Back(bool &exit)
{
    bool canGoBack = false;

    bool exitOnBack = false;
    Config.GetProperty("exitOnFirstPageBack", exitOnBack);
    exit = false;

    if(PageChain.size() > 1)
    {
        page->Stop();
        canGoBack = true;
    }
    else if(PageChain.size() == 1 && exitOnBack)
    {
        page->Stop();
        exit = true;
        canGoBack = true;
    }

    return canGoBack;
}


RetroFE::RETROFE_STATE RetroFE::ProcessUserInput()
{
    SDL_Event e;
    bool exit = false;
    RETROFE_STATE state = RETROFE_IDLE;

    if (SDL_PollEvent(&e) == 0) return state;

    if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
    {
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        InactiveKeyTime = 0;
        AttractMode = false;

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



Page *RetroFE::LoadPage(std::string collectionName)
{
    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Creating page for collection " + collectionName);

    Page *page = NULL;
    std::vector<Item *> *collection = new std::vector<Item *>(); // the page will deallocate this once its done
    MenuParser mp;

    mp.GetMenuItems(&CollectionDB, collectionName, *collection);
    CollectionDB.GetCollection(collectionName, *collection);

    //todo: handle this in a more esthetically pleasing way instead of crashing
    if(collection->size() == 0)
    {
        Logger::Write(Logger::ZONE_WARNING, "RetroFE", "No list items found for collection " + collectionName);
    }
    else
    {
        std::string layoutKeyName = "collections." + collectionName + ".layout";
        std::string layoutName = "Default 16x9";

        if(!Config.GetProperty(layoutKeyName, layoutName))
        {
            Config.GetProperty("layout", layoutName);
        }


        if(PageChain.size() > 0)
        {
            Page *oldPage = PageChain.back();

            if(oldPage)
            {
                oldPage->FreeGraphicsMemory();
            }
        }

        PageBuilder pb(layoutName, collectionName, Config, &FC);
        page = pb.BuildPage();
        page->SetItems(collection);
        page->Start();

        if(page)
        {
            PageChain.push_back(page);
        }
    }

    return page;
}
