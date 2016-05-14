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

#include "Page.h"
#include "ComponentItemBinding.h"
#include "Component/Component.h"
#include "../Collection/CollectionInfo.h"
#include "Component/Text.h"
#include "../Utility/Log.h"
#include "Component/ScrollingList.h"
#include "../Sound/Sound.h"
#include "ComponentItemBindingBuilder.h"
#include <algorithm>
#include <sstream>

Page::Page(Configuration &config)
    : config_(config)
    , activeMenu_(NULL)
    , menuDepth_(0)
    , scrollActive_(false)
    , selectedItem_(NULL)
    , textStatusComponent_(NULL)
    , selectedItemChanged_(false)
    , playlistChanged_(false)
    , loadSoundChunk_(NULL)
    , unloadSoundChunk_(NULL)
    , highlightSoundChunk_(NULL)
    , selectSoundChunk_(NULL)
    , minShowTime_(0)
{
}

Page::~Page()
{
}


void Page::DeInitialize()
{
    MenuVector_T::iterator it = menus_.begin();
    while(it != menus_.end())
    {
        ScrollingList *menu = *it;
        menu->removeComponentForNotifications(this);
        menus_.erase(it);
        delete menu;
        it = menus_.begin();
    }

    for(unsigned int i = 0; i < sizeof(LayerComponents)/sizeof(LayerComponents[0]); ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            delete *it;
        }

        LayerComponents[i].clear();
    }


    if(loadSoundChunk_)
    {
        delete loadSoundChunk_;
        loadSoundChunk_ = NULL;
    }

    if(unloadSoundChunk_)
    {
        delete unloadSoundChunk_;
        unloadSoundChunk_ = NULL;
    }


    if(highlightSoundChunk_)
    {
        delete highlightSoundChunk_;
        highlightSoundChunk_ = NULL;
    }

    if(selectSoundChunk_)
    {
        delete selectSoundChunk_;
        selectSoundChunk_ = NULL;
    }
    CollectionVector_T::iterator itc = collections_.begin();

    while(itc != collections_.end()) 
    {
        itc->collection->Save();

        if(itc->collection)
        {
            delete itc->collection;
        }
        collections_.erase(itc);
        itc = collections_.begin();
    }
}


bool Page::isMenusFull()
{
  return (menuDepth_ > menus_.size());
}

void Page::setLoadSound(Sound *chunk)
{
  loadSoundChunk_ = chunk;
}
void Page::setUnloadSound(Sound *chunk)
{
  unloadSoundChunk_ = chunk;
}
void Page::setHighlightSound(Sound *chunk)
{
  highlightSoundChunk_ = chunk;
}
void Page::setSelectSound(Sound *chunk)
{
  selectSoundChunk_ = chunk;
}
void Page::onNewItemSelected(Item *item)
{
    selectedItem_ = item;
    selectedItemChanged_ = true;
}

void Page::pushMenu(ScrollingList *s)
{
    menus_.push_back(s);

    if(s)
    {
        s->addComponentForNotifications(this);
    }
}

unsigned int Page::getMenuDepth()
{
    return menuDepth_;
}

void Page::setStatusTextComponent(Text *t)
{
    textStatusComponent_ = t;
}

bool Page::addComponent(Component *c)
{
    bool retVal = false;

    unsigned int layer = c->baseViewInfo.Layer;


    if(layer < NUM_LAYERS)
    {
        LayerComponents[layer].push_back(c);

        retVal = true;
    }
    else
    {
        std::stringstream ss;
        ss << "Component layer too large Layer: " << layer;
        Logger::write(Logger::ZONE_ERROR, "Page", ss.str());
    }

    return retVal;
}

bool Page::isMenuIdle()
{
    bool idle = true;

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        ScrollingList *menu = *it;

        if(!menu->isIdle())
        {
            idle = false;
            break;
        }
    }
    return idle;
}

bool Page::isIdle()
{
    bool idle = isMenuIdle();
    
    for(unsigned int i = 0; i < NUM_LAYERS && idle; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end() && idle; ++it)
        {
            idle = (*it)->isIdle();
        }
    }

    return idle;
}


bool Page::isHidden()
{
    bool hidden = true;

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        ScrollingList *menu = *it;

        if(!menu->isHidden())
        {
            hidden = false;
            break;
        }
    }


    for(unsigned int i = 0; hidden && i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); hidden && it != LayerComponents[i].end(); ++it)
        {
            hidden = (*it)->isHidden();
        }
    }

    return hidden;
}

void Page::start()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->triggerEnterEvent();
    }

    if(loadSoundChunk_)
    {
        loadSoundChunk_->play();
    }

    startComponents();
}


void Page::startComponents()
{
    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->triggerEnterEvent();
        }
    }
}

void Page::stop()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->triggerExitEvent();
    }

    if(unloadSoundChunk_)
    {
        unloadSoundChunk_->play();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->triggerExitEvent();
        }
    }
}


Item *Page::getSelectedItem()
{
    return selectedItem_;
}

Item *Page::getSelectedItem(int offset)
{
    return activeMenu_->getItemByOffset(offset);
}


void Page::removeSelectedItem()
{
    /*
    //todo: change method to RemoveItem() and pass in SelectedItem
    if(Menu)
    {
        Menu->RemoveSelectedItem();
    }
    */
    selectedItem_ = NULL;

}

void Page::setScrollOffsetIndex(unsigned int i)
{
    if(!activeMenu_) return;

    activeMenu_->setScrollOffsetIndex(i);
}

unsigned int Page::getScrollOffsetIndex()
{
    if(!activeMenu_) return -1;

    return activeMenu_->getScrollOffsetIndex();
}

void Page::setMinShowTime(float value)
{
    minShowTime_ = value;
}

float Page::getMinShowTime()
{
    return minShowTime_;
}

void Page::playlistChange()
{
    if(activeMenu_)
    {
        activeMenu_->triggerPlaylistChangeEvent(playlist_->first);
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->triggerPlaylistChangeEvent(playlist_->first);
        }
    }
}

void Page::highlight()
{
    Item *item = selectedItem_;

    if(!item) return;
    if(activeMenu_)
    {
        activeMenu_->triggerHighlightEvent();
        activeMenu_->scrollActive = scrollActive_;
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->triggerHighlightEvent();
            (*it)->scrollActive = scrollActive_;
        }
    }
}


void Page::setScrolling(ScrollDirection direction)
{
    ScrollingList::ScrollDirection menuDirection;
    bool prevScrollActive = scrollActive_;

    switch(direction)
    {
    case ScrollDirectionForward:
        menuDirection = ScrollingList::ScrollDirectionForward;
        scrollActive_ = true;
        break;
    case ScrollDirectionBack:
        menuDirection = ScrollingList::ScrollDirectionBack;
        scrollActive_ = true;
        break;
    case ScrollDirectionIdle:
    default:
        menuDirection = ScrollingList::ScrollDirectionIdle;
        scrollActive_ = false;
        break;
    }

    if(!prevScrollActive && scrollActive_ && highlightSoundChunk_)
    {
        highlightSoundChunk_->play();
    }


    activeMenu_->setScrollDirection(menuDirection);
}

bool Page::isHorizontalScroll()
{
   if(!activeMenu_) { return false; }

   return activeMenu_->horizontalScroll;
}

void Page::pageScroll(ScrollDirection direction)
{
    if(activeMenu_)
    {
        if(direction == ScrollDirectionForward)
        {
            activeMenu_->pageDown();
        }
        if(direction == ScrollDirectionBack)
        {
            activeMenu_->pageUp();
        }
    }
}

void Page::selectRandom()
{
    if(activeMenu_) activeMenu_->random(); 
}

void Page::letterScroll(ScrollDirection direction)
{
    if(activeMenu_)
    {
        if(direction == ScrollDirectionForward)
        {
            activeMenu_->letterDown();
        }
        if(direction == ScrollDirectionBack)
        {
            activeMenu_->letterUp();
        }
    }
}

unsigned int Page::getCollectionSize()
{
    return activeMenu_->getSize();
}

unsigned int Page::getSelectedIndex()
{
    return activeMenu_->getSelectedIndex();
}


bool Page::pushCollection(CollectionInfo *collection)
{

    int menuExitIndex = -1;
    int menuEnterIndex = -1;

    if(activeMenu_)
    {
        activeMenu_->triggerMenuExitEvent();
    }

    if(menuDepth_ > 0)
    {
        menuExitIndex = menuDepth_ - 1;
    }

    // grow the menu as needed
    if(menus_.size() >= menuDepth_ && activeMenu_)
    {
        activeMenu_ = new ScrollingList(*activeMenu_);
        activeMenu_->forceIdle();
        pushMenu(activeMenu_);
    }


    activeMenu_ = menus_[menuDepth_];
    activeMenu_->collectionName = collection->name;
    activeMenu_->setItems(&collection->items);
    activeMenu_->triggerMenuEnterEvent();

    // build the collection info instance
    MenuInfo_S info;
    info.collection = collection;
    info.menu = activeMenu_;
    info.playlist = collection->playlists.begin();
    info.queueDelete = false;
    collections_.push_back(info);

    playlist_ = info.playlist;
    playlistChanged_ = true;

    if(menuDepth_ < menus_.size())
    {
        menuEnterIndex = menuDepth_;
        menuDepth_++;
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->collectionName = collection->name;
            if(menuEnterIndex >= 0)
            {
                (*it)->triggerMenuEnterEvent(menuEnterIndex);
            }

            if(menuExitIndex >= 0)
            {
                (*it)->triggerMenuExitEvent(menuExitIndex);
            }
        }
    }

    return true;
}

bool Page::popCollection()
{
    int menuExitIndex = -1;
    int menuEnterIndex = -1;

    if(!activeMenu_) return false;
    if(menuDepth_ <= 1) return false;
    if(collections_.size() <= 1) return false;

    // queue the collection for deletion
    MenuInfo_S *info = &collections_.back();
    info->queueDelete = true;
    deleteCollections_.push_back(*info);

    // get the next collection off of the stack
    collections_.pop_back();
    info = &collections_.back();
    playlist_ = info->playlist;
    playlistChanged_ = true;

    if(activeMenu_)
    {
        activeMenu_->triggerMenuExitEvent();

    }

    menuDepth_--;
    menuExitIndex = menuDepth_;
    menuEnterIndex = menuExitIndex - 1;
    activeMenu_ = menus_[menuDepth_ - 1];
    if(activeMenu_)
    {
        activeMenu_->triggerMenuEnterEvent();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->collectionName = info->collection->name;

            if(menuEnterIndex >= 0)
            {
                (*it)->triggerMenuEnterEvent(menuEnterIndex);
            }

            if(menuExitIndex >= 0)
            {
                (*it)->triggerMenuExitEvent(menuExitIndex);
            }
        }
    }

    return true;
}

void Page::nextPlaylist()
{
    MenuInfo_S &info = collections_.back();
    info.collection->Save();
    unsigned int numlists = info.collection->playlists.size();

    for(unsigned int i = 0; i <= numlists; ++i)
    {
        playlist_++;
        // wrap
        if(playlist_ == info.collection->playlists.end()) playlist_ = info.collection->playlists.begin();
       
        // find the first playlist
        if(playlist_->second->size() != 0) break;
    }

    activeMenu_->setItems(playlist_->second);
    activeMenu_->triggerMenuEnterEvent();
    playlistChanged_ = true;
}

void Page::favPlaylist()
{
    MenuInfo_S &info = collections_.back();
    info.collection->Save();
    unsigned int numlists = info.collection->playlists.size();

    // Store current playlist
    CollectionInfo::Playlists_T::iterator playlist_store = playlist_;

    for(unsigned int i = 0; i <= numlists; ++i)
    {
        playlist_++;
        // wrap
        if(playlist_ == info.collection->playlists.end()) playlist_ = info.collection->playlists.begin();
        
        // find the first playlist
        if(playlist_->second->size() != 0 && playlist_->first == "favorites") break;
    }

    // Do not change playlist if favorites does not exist or if it's empty
    if ( playlist_->second->size() == 0 || playlist_->first != "favorites")
      playlist_ = playlist_store;

    activeMenu_->setItems(playlist_->second);
    activeMenu_->triggerMenuEnterEvent();
    playlistChanged_ = true;
}

void Page::update(float dt)
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        ScrollingList *menu = *it;

        menu->update(dt);
    }
    
    if(playlistChanged_)
    {
        playlistChange();
        playlistChanged_ = false;
    }

    if(selectedItemChanged_ && !scrollActive_)
    {
        highlight();
        selectedItemChanged_ = false;
    }

    if(textStatusComponent_)
    {
    	std::string status;
    	config_.setProperty("status", status);
        textStatusComponent_->setText(status);
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            if(*it) (*it)->update(dt);
        }
    }

    // many nodes still have handles on the collection info. We need to delete
    // them once everything is done using them
    std::list<MenuInfo_S>::iterator del = deleteCollections_.begin();

    while(del != deleteCollections_.end())
    {
        MenuInfo_S &info = *del;
        if(info.queueDelete && info.menu && info.menu->isIdle())
        {   
            std::list<MenuInfo_S>::iterator next = del;
            ++next;

            if(info.collection) 
            {
                info.collection->Save();
                delete info.collection;
            }
            deleteCollections_.erase(del);
            del = next;
        }
        else
        {
            ++del;
        }
    }
}

void Page::draw()
{
    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            if(*it && (*it)->baseViewInfo.Layer == i) (*it)->draw();
        }

        for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
        {
            ScrollingList *menu = *it;
            menu->draw(i);
        }
    }

}

void Page::removePlaylist()
{
    if(!selectedItem_) return;
    if(!selectedItem_->leaf) return;

    MenuInfo_S &info = collections_.back();
    CollectionInfo *collection = info.collection;

    std::vector<Item *> *items = collection->playlists["favorites"];
    std::vector<Item *>::iterator it = std::find(items->begin(), items->end(), selectedItem_);

    if(it != items->end())
    {
        items->erase(it);
        collection->saveRequest = true;

        if(activeMenu_) 
        {
            activeMenu_->deallocateSpritePoints();
            activeMenu_->allocateSpritePoints();
        }
    }
}

void Page::addPlaylist()
{
    if(!selectedItem_) return;
    if(!selectedItem_->leaf) return;

    MenuInfo_S &info = collections_.back();
    CollectionInfo *collection = info.collection;

    std::vector<Item *> *items = collection->playlists["favorites"];
    if(playlist_->first != "favorites" && std::find(items->begin(), items->end(), selectedItem_) == items->end())
    {
        items->push_back(selectedItem_);
        collection->saveRequest = true;
        if(activeMenu_) 
        {
            activeMenu_->deallocateSpritePoints();
            activeMenu_->allocateSpritePoints();
        }
    }
}

std::string Page::getCollectionName()
{
    if(collections_.size() == 0) return "";

    MenuInfo_S &info = collections_.back();
    return info.collection->name;

}

void Page::freeGraphicsMemory()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->freeGraphicsMemory();
    }

    if(loadSoundChunk_) loadSoundChunk_->free();
    if(unloadSoundChunk_) unloadSoundChunk_->free();
    if(highlightSoundChunk_) highlightSoundChunk_->free();
    if(selectSoundChunk_) selectSoundChunk_->free();

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->freeGraphicsMemory();
        }
    }
}

void Page::allocateGraphicsMemory()
{
    Logger::write(Logger::ZONE_DEBUG, "Page", "Allocating graphics memory");

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        ScrollingList *menu = *it;

        menu->allocateGraphicsMemory();
    }

    if(loadSoundChunk_) loadSoundChunk_->allocate();
    if(unloadSoundChunk_) unloadSoundChunk_->allocate();
    if(highlightSoundChunk_) highlightSoundChunk_->allocate();
    if(selectSoundChunk_) selectSoundChunk_->allocate();

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->allocateGraphicsMemory();
        }
    }
    Logger::write(Logger::ZONE_DEBUG, "Page", "Allocate graphics memory complete");
}

void Page::launchEnter()
{
    if(activeMenu_)
    {
        activeMenu_->launchEnter();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->launchEnter();
        }
    }
}

void Page::launchExit()
{
    if(activeMenu_)
    {
        activeMenu_->launchExit();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->launchExit();
        }
    }
}
