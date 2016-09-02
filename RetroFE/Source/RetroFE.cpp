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
#include "Graphics/Component/Video.h"
#include "Video/VideoFactory.h"
#include <vector>
#include <string>
#include <sstream>
#include <dirent.h>

#ifdef __linux
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>
#endif

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
    , lastLaunchReturnTime_(0)
    , keyLastTime_(0)
    , keyDelayTime_(.3f)
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
    lastLaunchReturnTime_ = currentTime_;

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
    }
}

bool RetroFE::deInitialize()
{
    bool retVal = true;
    freeGraphicsMemory();

    if(currentPage_)
    {
        currentPage_->DeInitialize();
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
    Video::setEnabled(videoEnable);


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
    currentPage_        = loadSplashPage();
    state               = RETROFE_ENTER;
    bool splashMode     = true;
    bool exitSplashMode = false;

    Launcher l(*this, config_);
    preloadTime = static_cast<float>(SDL_GetTicks()) / 1000;

    while (running)
    {
        float lastTime = 0;
        float deltaTime = 0;
        SDL_Event e;
        if (SDL_PollEvent(&e))
        {
            if(input_.update(e))
            {
                exitSplashMode = true;
                attract_.reset();
            }
        }

        if(!currentPage_)
        {
            Logger::write(Logger::ZONE_WARNING, "RetroFE", "Could not load page");
            running = false;
            break;
        }
        switch(state)
        {
        case RETROFE_IDLE:
            if(currentPage_ && !splashMode)
            {

                // account for when returning from a menu and the previous key was still "stuck"
                if(lastLaunchReturnTime_ == 0 || (currentTime_ - lastLaunchReturnTime_ > .3))
                {
                    state = processUserInput(currentPage_);
                    lastLaunchReturnTime_ = 0;
                }
            }

            if((initialized || initializeError) && splashMode && (exitSplashMode || (currentPage_->getMinShowTime() <= (currentTime_ - preloadTime) && !(currentPage_->isPlaying()))))
            {
                SDL_WaitThread(initializeThread, &initializeStatus);

                if(initializeError)
                {
                    state = RETROFE_QUIT_REQUEST;
                    break;
                }

                currentPage_->stop();
                state = RETROFE_SPLASH_EXIT;

            }
            break;

        case RETROFE_LOAD_ART:
            currentPage_->start();
            state = RETROFE_ENTER;
            break;

        case RETROFE_ENTER:
            if(currentPage_->isIdle())
            {
                state = RETROFE_IDLE;
            }
            break;

        case RETROFE_SPLASH_EXIT:
            if(currentPage_->isIdle())
            {
                // delete the splash screen and use the standard menu
                currentPage_->DeInitialize();
                delete currentPage_;

                currentPage_ = loadPage();
                splashMode = false;
                if(currentPage_)
                {
                    std::string firstCollection = "Main";
                    bool menuSort = true;

                    config_.getProperty("firstCollection", firstCollection);
                    config_.getProperty("collections." + firstCollection + ".list.menuSort", menuSort);
                    config_.setProperty("currentCollection", firstCollection);
                    CollectionInfo *info = getCollection(firstCollection);
                    MenuParser mp;

                    mp.buildMenuItems(info, menuSort);

                    currentPage_->pushCollection(info);
                    currentPage_->onNewItemSelected();
                    currentPage_->reallocateMenuSpritePoints();

                    state = RETROFE_LOAD_ART;
                }
                else
                {
                    state = RETROFE_QUIT_REQUEST;
                }
            }
            break;

        case RETROFE_HIGHLIGHT_REQUEST:
            currentPage_->highlightExit();
            currentPage_->setScrolling(Page::ScrollDirectionIdle);
            state = RETROFE_HIGHLIGHT_EXIT;
            break;

        case RETROFE_HIGHLIGHT_EXIT:
            if ( processUserInput(currentPage_) == RETROFE_HIGHLIGHT_REQUEST)
            {
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            else if ((currentPage_->isGraphicsIdle() && currentPage_->isMenuScrolling()) ||
                     (currentPage_->isIdle()))
            {
                currentPage_->onNewItemSelected();
                state = RETROFE_HIGHLIGHT_LOAD_ART;
            }
            break;

        case RETROFE_HIGHLIGHT_LOAD_ART:
            if ( processUserInput(currentPage_) == RETROFE_HIGHLIGHT_REQUEST)
            {
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            else if ((currentPage_->isGraphicsIdle() && currentPage_->isMenuScrolling()) ||
                     (currentPage_->isIdle()))
            {
                currentPage_->highlightEnter();
                state = RETROFE_HIGHLIGHT_ENTER;
            }
            break;

        case RETROFE_HIGHLIGHT_ENTER:
            if ( processUserInput(currentPage_) == RETROFE_HIGHLIGHT_REQUEST)
            {
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            else if (currentPage_->isGraphicsIdle())
            {
                state = RETROFE_IDLE;
            }
            break;

        case RETROFE_NEXT_PAGE_REQUEST:
            currentPage_->exitMenu();
            state = RETROFE_NEXT_PAGE_MENU_EXIT;
            break;

        case RETROFE_NEXT_PAGE_MENU_EXIT:
            if(currentPage_->isIdle())
            {
                // Load new layout if available
                std::string layoutName;
                config_.getProperty("layout", layoutName);
                PageBuilder pb(layoutName, "layout", config_, &fontcache_);
                Page *page = pb.buildPage( nextPageItem_->name);
                std::string nextPageName = nextPageItem_->name;
                if(page)
                {
                    currentPage_->freeGraphicsMemory();
                    pages_.push( currentPage_ );
                    currentPage_ = page;
                }

                bool menuSort = true;
                config_.setProperty("currentCollection", nextPageName);
                config_.getProperty("collections." + nextPageName + ".list.menuSort", menuSort);

                CollectionInfo *info = getCollection(nextPageName);

                MenuParser mp;
                mp.buildMenuItems(info, menuSort);
                currentPage_->pushCollection(info);

                bool rememberMenu = false;
                config_.getProperty("rememberMenu", rememberMenu);
                bool autoFavorites = true;
                config_.getProperty("autoFavorites", autoFavorites);

                if (rememberMenu && lastMenuPlaylists_.find(nextPageName) != lastMenuPlaylists_.end())
                {
                  currentPage_->selectPlaylist(lastMenuPlaylists_[nextPageName]); // Switch to last playlist
                }
                else if (autoFavorites)
                {
                  currentPage_->selectPlaylist("favorites"); // Switch to favorites playlist
                }

                if(rememberMenu && lastMenuOffsets_.find(nextPageName) != lastMenuOffsets_.end())
                {
                    currentPage_->setScrollOffsetIndex(lastMenuOffsets_[nextPageName]);
                }

                currentPage_->onNewItemSelected();
                currentPage_->reallocateMenuSpritePoints();

                state = RETROFE_NEXT_PAGE_MENU_LOAD_ART;

             }
             break;

        case RETROFE_NEXT_PAGE_MENU_LOAD_ART:
            if (currentPage_->getMenuDepth() != 1 )
            {
                currentPage_->enterMenu();
            }
            else
            {
                currentPage_->start();
            }
            state = RETROFE_NEXT_PAGE_MENU_ENTER;
            break;

        case RETROFE_NEXT_PAGE_MENU_ENTER:
            if(currentPage_->isIdle())
            {
              state = RETROFE_IDLE;
            }
          break;

        case RETROFE_LAUNCH_REQUEST:
            nextPageItem_ = currentPage_->getSelectedItem();
            launchEnter();
            l.run(nextPageItem_->collectionInfo->name, nextPageItem_);
            launchExit();
            state = RETROFE_IDLE;
            break;

        case RETROFE_BACK_REQUEST:
            if (currentPage_->getMenuDepth() == 1 )
            {
                currentPage_->stop();
            }
            else
            {
                currentPage_->exitMenu();
            }
            state = RETROFE_BACK_MENU_EXIT;
            break;

        case RETROFE_BACK_MENU_EXIT:
            if(currentPage_->isIdle())
            {
                lastMenuOffsets_[currentPage_->getCollectionName()]   = currentPage_->getScrollOffsetIndex();
                lastMenuPlaylists_[currentPage_->getCollectionName()] = currentPage_->getPlaylistName();
                if (currentPage_->getMenuDepth() == 1)
                {
                    currentPage_->DeInitialize();
                    delete currentPage_;
                    currentPage_ = pages_.top();
                    pages_.pop();
                    currentPage_->allocateGraphicsMemory();
                }
                else
                {
                    currentPage_->popCollection();
                }
                config_.setProperty("currentCollection", currentPage_->getCollectionName());
                currentPage_->onNewItemSelected();
                currentPage_->reallocateMenuSpritePoints();
                state = RETROFE_BACK_MENU_LOAD_ART;
            }
            break;

        case RETROFE_BACK_MENU_LOAD_ART:
            currentPage_->enterMenu();
            state = RETROFE_BACK_MENU_ENTER;
            break;

        case RETROFE_BACK_MENU_ENTER:
            if(currentPage_->isIdle())
            {
                currentPage_->cleanup();
                state = RETROFE_IDLE;
            }
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
            if(currentPage_->isGraphicsIdle())
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
                if (!splashMode)
                {
                    attract_.update(deltaTime, *currentPage_);
                }
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

    if(currentPage_->getMenuDepth() <= 1 && pages_.empty())
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
    bool exit = false;
    RETROFE_STATE state = RETROFE_IDLE;

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

    if(page->isMenuIdle())
    {

        if (!input_.keystate(UserInput::KeyCodePageUp) &&
            !input_.keystate(UserInput::KeyCodePageDown) &&
            !input_.keystate(UserInput::KeyCodeLetterUp) &&
            !input_.keystate(UserInput::KeyCodeLetterDown) &&
            !input_.keystate(UserInput::KeyCodeNextPlaylist) &&
            !input_.keystate(UserInput::KeyCodeAddPlaylist) &&
            !input_.keystate(UserInput::KeyCodeRemovePlaylist) &&
            !input_.keystate(UserInput::KeyCodeRandom))
        {
            keyLastTime_ = 0;
            keyDelayTime_= 0.3f;
        }

        else if((currentTime_ - keyLastTime_) > keyDelayTime_ || keyLastTime_ == 0)
        {
            keyLastTime_ = currentTime_;
            keyDelayTime_-= .05f;
            if(keyDelayTime_< 0.1f) keyDelayTime_= 0.1f;

            if (input_.keystate(UserInput::KeyCodePageUp))
            {
                page->pageScroll(Page::ScrollDirectionBack);
                page->reallocateMenuSpritePoints();
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            if (input_.keystate(UserInput::KeyCodePageDown))
            {
                page->pageScroll(Page::ScrollDirectionForward);
                page->reallocateMenuSpritePoints();
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            if (input_.keystate(UserInput::KeyCodeLetterUp))
            {
                page->letterScroll(Page::ScrollDirectionBack);
                page->reallocateMenuSpritePoints();
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            if (input_.keystate(UserInput::KeyCodeLetterDown))
            {
                page->letterScroll(Page::ScrollDirectionForward);
                page->reallocateMenuSpritePoints();
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            if(input_.newKeyPressed(UserInput::KeyCodeNextPlaylist))
            {
                page->nextPlaylist();
                page->reallocateMenuSpritePoints();
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
            if(input_.newKeyPressed(UserInput::KeyCodeRemovePlaylist))
            {
                page->removePlaylist();
                page->onNewItemSelected();
                page->reallocateMenuSpritePoints();
            }
            if(input_.newKeyPressed(UserInput::KeyCodeAddPlaylist))
            {
                page->addPlaylist();
                page->reallocateMenuSpritePoints();
            }
            if(input_.keystate(UserInput::KeyCodeRandom))
            {
                page->selectRandom();
                page->reallocateMenuSpritePoints();
                state = RETROFE_HIGHLIGHT_REQUEST;
            }
        }

        if (input_.keystate(UserInput::KeyCodeAdminMode))
        {
            //todo: add admin mode support
        }
        if (input_.keystate(UserInput::KeyCodeSelect))
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
                    state = RETROFE_NEXT_PAGE_REQUEST;
                }
            }
        }

        if (input_.keystate(UserInput::KeyCodeBack))
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
    }

    if(!input_.keystate(UserInput::KeyCodeUp) &&
       !input_.keystate(UserInput::KeyCodeLeft) &&
       !input_.keystate(UserInput::KeyCodeDown) &&
       !input_.keystate(UserInput::KeyCodeRight) &&
       !input_.keystate(UserInput::KeyCodePageUp) &&
       !input_.keystate(UserInput::KeyCodePageDown) &&
       !attract_.isActive())
    {
        if (page->isMenuScrolling())
            state = RETROFE_HIGHLIGHT_REQUEST;
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


CollectionInfo *RetroFE::getCollection(std::string collectionName)
{
    // the page will deallocate this once its done

    CollectionInfoBuilder cib(config_, *metadb_);
    CollectionInfo *collection = cib.buildCollection(collectionName);
    DIR *dp;
    struct dirent *dirp;

    std::string path = Utils::combinePath(Configuration::absolutePath, "collections", collectionName);
    dp = opendir(path.c_str());

    while((dirp = readdir(dp)) != NULL)
    {
        std::string file = dirp->d_name;

        size_t position = file.find_last_of(".");
        std::string basename = (std::string::npos == position)? file : file.substr(0, position);

        std::string comparator = ".sub";
        int start = file.length() - comparator.length();

        if(start >= 0)
        {
            if(file.compare(start, comparator.length(), comparator) == 0)
            {
                Logger::write(Logger::ZONE_INFO, "RetroFE", "Loading subcollection into menu: " + basename);

                CollectionInfo *subcollection = cib.buildCollection(basename, collectionName);
                collection->addSubcollection(subcollection);
            }
        }
    }

    collection->sortItems();
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
