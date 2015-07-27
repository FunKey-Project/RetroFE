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
#include "Utility/Utils.h"
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
    : initialized(false)
    , initializeError(false)
    , initializeThread(NULL)
    , config_(c)
    , db_(NULL)
    , metadb_(NULL)
    , input_(config_)
    , currentPage_(NULL)
    , keyInputDisable_(0)
    , currentTime_(0)
{
}

RetroFE::~RetroFE()
{
    deInitialize();
}

void RetroFE::render()
{
    SDL_LockMutex(SDL::getMutex());
    SDL_SetRenderDrawColor(SDL::getRenderer(), 0x0, 0x0, 0x00, 0xFF);
    SDL_RenderClear(SDL::getRenderer());

    if(currentPage_)
    {
        currentPage_->draw();
    }

    SDL_RenderPresent(SDL::getRenderer());
    SDL_UnlockMutex(SDL::getMutex());
}

int RetroFE::initialize(void *context)
{
    RetroFE *instance = static_cast<RetroFE *>(context);

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Initializing");

    if(!instance->input_.initialize()) 
    { 
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not initialize user controls");
        instance->initializeError = true;
        return -1;
    }

    instance->db_ = new DB(Utils::combinePath(Configuration::absolutePath, "meta.db"));

    if(!instance->db_->initialize())
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not initialize database");
        instance->initializeError = true;
        return -1;
    }

    instance->metadb_ = new MetadataDatabase(*(instance->db_), instance->config_);

    if(!instance->metadb_->initialize())
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not initialize meta database");
        instance->initializeError = true;
        return -1;
    }

    instance->initialized = true;
    return 0;
}

void RetroFE::launchEnter()
{
    if(currentPage_)
    {
        currentPage_->launchEnter();
    }

    SDL_SetWindowGrab(SDL::getWindow(), SDL_FALSE);

}

void RetroFE::launchExit()
{
    SDL_RestoreWindow(SDL::getWindow());
    SDL_RaiseWindow(SDL::getWindow());
    SDL_SetWindowGrab(SDL::getWindow(), SDL_TRUE);
    input_.resetStates();
    attract_.reset();

    currentTime_ = static_cast<float>(SDL_GetTicks()) / 1000;
    if(currentPage_)
    {
        currentPage_->launchExit();
    }

}

void RetroFE::freeGraphicsMemory()
{
    if(currentPage_)
    {
        currentPage_->freeGraphicsMemory();
    }
    fontcache_.deInitialize();

    SDL::deInitialize();
}

void RetroFE::allocateGraphicsMemory()
{
    SDL::initialize(config_);

    fontcache_.initialize();

    if(currentPage_)
    {
        currentPage_->allocateGraphicsMemory();
        currentPage_->start();
    }
}

bool RetroFE::deInitialize()
{
    bool retVal = true;
    freeGraphicsMemory();

    if(currentPage_)
    {
        delete currentPage_;
        currentPage_ = NULL;
    }
    if(metadb_)
    {
        delete metadb_;
        metadb_ = NULL;
    }

    if(db_)
    {
        delete db_;
        db_ = NULL;
    }

    initialized = false;
    //todo: handle video deallocation

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Exiting");

    return retVal;
}

void RetroFE::run()
{
    if(!SDL::initialize(config_)) return;

    fontcache_.initialize();
    float preloadTime = 0;
    bool videoEnable = true;
    int videoLoop = 0;
    config_.getProperty("videoEnable", videoEnable);
    config_.getProperty("videoLoop", videoLoop);

    VideoFactory::setEnabled(videoEnable);
    VideoFactory::setNumLoops(videoLoop);
    VideoFactory::createVideo(); // pre-initialize the gstreamer engine


    initializeThread = SDL_CreateThread(initialize, "RetroFEInit", (void *)this);

    if(!initializeThread)
    {
        Logger::write(Logger::ZONE_INFO, "RetroFE", "Could not initialize RetroFE");
        return;
    }

    int attractModeTime = 0;
    std::string firstCollection = "Main";
    bool running = true;
    RETROFE_STATE state = RETROFE_NEW;

    config_.getProperty("attractModeTime", attractModeTime);
    config_.getProperty("firstCollection", firstCollection);

    attract_.idleTime = static_cast<float>(attractModeTime);

    int initializeStatus = 0;

    // load the initial splash screen, unload it once it is complete
    currentPage_ = loadSplashPage();
    bool splashMode = true;

    Launcher l(*this, config_);
    preloadTime = static_cast<float>(SDL_GetTicks()) / 1000;

    while (running)
    {
        float lastTime = 0;
        float deltaTime = 0;

        if(!currentPage_)
        {
            Logger::write(Logger::ZONE_WARNING, "RetroFE", "Could not load page");
            running = false;
            break;
        }
        // todo: This could be transformed to use the state design pattern.
        switch(state)
        {
        case RETROFE_IDLE:
            if(currentPage_ && !splashMode)
            {
                state = processUserInput(currentPage_);
            }
            else
            {
                // read and discard SDL input to prevent windows from balking at us
                SDL_Event e;
                (void)SDL_PollEvent(&e);
            }

            if((initialized || initializeError) && splashMode && currentPage_->getMinShowTime() <= (currentTime_ - preloadTime))
            {
                SDL_WaitThread(initializeThread, &initializeStatus);

                if(initializeError)
                {
                    state = RETROFE_QUIT_REQUEST;
                    break;
                }

                // delete the splash screen and use the standard menu
                delete currentPage_;

                currentPage_ = loadPage();
                splashMode = false;
                if(currentPage_)
                {
                    std::string firstCollection = "Main";
                    bool menuSort = true;

                    config_.getProperty("firstCollection", firstCollection);
                    config_.getProperty("collections." + firstCollection + ".list.menuSort", menuSort);

                    currentPage_->start();
                    config_.setProperty("currentCollection", firstCollection);
                    CollectionInfo *info = getCollection(firstCollection, "include");
                    MenuParser mp;

                    CollectionInfoBuilder cib(config_, *metadb_);
                    mp.buildMenuItems(info, menuSort, cib);

                    currentPage_->pushCollection(info, false);
                }
                else
                {
                    state = RETROFE_QUIT_REQUEST;
                }

            }

            break;

        case RETROFE_NEXT_PAGE_REQUEST:
            if(currentPage_->isIdle())
            {
                state = RETROFE_NEW;
            }
            break;

        case RETROFE_LAUNCH_REQUEST:
            nextPageItem_ = currentPage_->getSelectedItem();
            l.run(nextPageItem_->collectionInfo->name, nextPageItem_);
            state = RETROFE_IDLE;
            break;

        case RETROFE_BACK_REQUEST:

            lastMenuOffsets_[currentPage_->getCollectionName()] = currentPage_->getScrollOffsetIndex();
            currentPage_->popCollection();
            config_.setProperty("currentCollection", currentPage_->getCollectionName());

            state = RETROFE_NEW;

            break;

        case RETROFE_NEW:
            if(currentPage_->isIdle())
            {
                state = RETROFE_IDLE;
            }
            break;

        case RETROFE_QUIT_REQUEST:
            currentPage_->stop();
            state = RETROFE_QUIT;
            break;

        case RETROFE_QUIT:
            if(currentPage_->isHidden())
            {
                running = false;
            }

            break;
        }

        // the logic below could be done in a helper method
        if(running)
        {
            lastTime = currentTime_;
            currentTime_ = static_cast<float>(SDL_GetTicks()) / 1000;

            if (currentTime_ < lastTime)
            {
                currentTime_ = lastTime;
            }

            deltaTime = currentTime_ - lastTime;
            double sleepTime = 1000.0/60.0 - deltaTime*1000;
            if(sleepTime > 0)
            {
                SDL_Delay(static_cast<unsigned int>(sleepTime));
            }

            if(currentPage_)
            {
                attract_.update(deltaTime, *currentPage_);
                currentPage_->update(deltaTime);
            }

            render();
        }
    }

}


bool RetroFE::back(bool &exit)
{
    bool canGoBack = false;
    bool exitOnBack = false;
    config_.getProperty("exitOnFirstPageBack", exitOnBack);
    exit = false;

    if(currentPage_->getMenuDepth() <= 1)
    {
        exit = exitOnBack;
    }
    else
    {
        canGoBack = true;
    }

    return canGoBack;
}


RetroFE::RETROFE_STATE RetroFE::processUserInput(Page *page)
{
    SDL_Event e;
    bool exit = false;
    RETROFE_STATE state = RETROFE_IDLE;
    if (SDL_PollEvent(&e) == 0) return state;
    bool rememberMenu = false;
    config_.getProperty("rememberMenu", rememberMenu);

    if(input_.update(e))
    {
        attract_.reset();

	    if(page->isHorizontalScroll())
        {
            if (input_.keystate(UserInput::KeyCodeLeft))
            {
                page->setScrolling(Page::ScrollDirectionBack);
            }
            if (input_.keystate(UserInput::KeyCodeRight))
            {
                page->setScrolling(Page::ScrollDirectionForward);
            } 
        }
        else
        { 
            if (input_.keystate(UserInput::KeyCodeUp))
            {
                page->setScrolling(Page::ScrollDirectionBack);
            }
            if (input_.keystate(UserInput::KeyCodeDown))
            {
                    page->setScrolling(Page::ScrollDirectionForward);
            }
        }
        if (input_.keystate(UserInput::KeyCodePageUp))
        {
            page->pageScroll(Page::ScrollDirectionBack);
        }
        if (input_.keystate(UserInput::KeyCodePageDown))
        {
            page->pageScroll(Page::ScrollDirectionForward);
        }
        if (input_.keystate(UserInput::KeyCodeLetterUp))
        {
            page->letterScroll(Page::ScrollDirectionBack);
        }
        if (input_.keystate(UserInput::KeyCodeLetterDown))
        {
            page->letterScroll(Page::ScrollDirectionForward);
        }

        if (input_.keystate(UserInput::KeyCodeAdminMode))
        {
            //todo: add admin mode support
        }
        if (input_.keystate(UserInput::KeyCodeSelect) && page->isMenuIdle())
        {
            nextPageItem_ = page->getSelectedItem();

            if(nextPageItem_)
            {
                if(nextPageItem_->leaf)
                {
                    state = RETROFE_LAUNCH_REQUEST;
                }
                else
                {
                	bool menuSort = true;
                    config_.setProperty("currentCollection", nextPageItem_->name);
                    config_.getProperty("collections." + nextPageItem_->name + ".list.menuSort", menuSort);

                    CollectionInfo *info = getCollection(nextPageItem_->name, "include");

                    MenuParser mp;
                    CollectionInfoBuilder cib(config_, *metadb_);
                    mp.buildMenuItems(info, menuSort, cib);
                    page->pushCollection(info, false);
                    listnames_.push_back("include");
                    listnames_.push_back("favorites");
                    listnameit_ = listnames_.begin();

                    if(rememberMenu && lastMenuOffsets_.find(nextPageItem_->name) != lastMenuOffsets_.end())
                    {
                        page->setScrollOffsetIndex(lastMenuOffsets_[nextPageItem_->name]);
                    }
                    
                    state = RETROFE_NEXT_PAGE_REQUEST;
                }
            }
        }
        if(input_.keystate(UserInput::KeyCodeNextPlaylist) && page->isMenuIdle())
        {
            nextPageItem_ = page->getSelectedItem();

            bool menuSort = true;
            config_.setProperty("currentCollection", nextPageItem_->name);
            config_.getProperty("collections." + nextPageItem_->name + ".list.menuSort", menuSort);

            listnameit_++;
            if(listnameit_ == listnames_.end()) listnameit_ = listnames_.begin(); 

            CollectionInfo *info = getCollection(currentPage_->getCollectionName(), *listnameit_);

            MenuParser mp;
            CollectionInfoBuilder cib(config_, *metadb_);
            mp.buildMenuItems(info, menuSort, cib);
            page->pushCollection(info, true);

            if(rememberMenu && lastMenuOffsets_.find(nextPageItem_->name) != lastMenuOffsets_.end())
            {
                page->setScrollOffsetIndex(lastMenuOffsets_[nextPageItem_->name]);
            }
            
            state = RETROFE_NEXT_PAGE_REQUEST;

        }

        if (input_.keystate(UserInput::KeyCodeBack) && page->isMenuIdle())
        {
            if(back(exit) || exit)
            {
                state = (exit) ? RETROFE_QUIT_REQUEST : RETROFE_BACK_REQUEST;
            }
        }

        if (input_.keystate(UserInput::KeyCodeQuit))
        {
            state = RETROFE_QUIT_REQUEST;
        }

        if(!input_.keystate(UserInput::KeyCodeUp) &&
                !input_.keystate(UserInput::KeyCodeLeft) &&
                !input_.keystate(UserInput::KeyCodeDown) &&
                !input_.keystate(UserInput::KeyCodeRight) &&
                !input_.keystate(UserInput::KeyCodePageUp) &&
                !input_.keystate(UserInput::KeyCodePageDown))
        {
            page->setScrolling(Page::ScrollDirectionIdle);
        }
    }

    return state;
}

Page *RetroFE::loadPage()
{
    std::string layoutName;

    config_.getProperty("layout", layoutName);

    PageBuilder pb(layoutName, "layout", config_, &fontcache_);
    Page *page = pb.buildPage();

    if(!page)
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not create page");
    }
    else
    {
        page->start();
    }

    return page;
}

Page *RetroFE::loadSplashPage()
{
    std::string layoutName;
    config_.getProperty("layout", layoutName);

    PageBuilder pb(layoutName, "splash", config_, &fontcache_);
    Page * page = pb.buildPage();
    page->start();

    return page;
}


CollectionInfo *RetroFE::getCollection(std::string collectionName, std::string listname)
{
    // the page will deallocate this once its done

    CollectionInfoBuilder cib(config_, *metadb_);
    CollectionInfo *collection = cib.buildCollection(collectionName, listname);

    return collection;
}

std::string RetroFE::getLayout(std::string collectionName)
{
    std::string layoutKeyName = "collections." + collectionName + ".layout";
    std::string layoutName = "Default 16x9";

    if(!config_.getProperty(layoutKeyName, layoutName))
    {
        config_.getProperty("layout", layoutName);
    }

    return layoutName;
}
