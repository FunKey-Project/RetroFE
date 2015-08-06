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

#include "MenuNotifierInterface.h"
#include "../Collection/CollectionInfo.h"

#include <map>
#include <string>
#include <list>
#include <vector>

class Component;
class Configuration;
class ScrollingList;
class Text;
class Item;
class Sound;

class Page : public MenuNotifierInterface
{
public:
    enum ScrollDirection
    {
        ScrollDirectionForward,
        ScrollDirectionBack,
        ScrollDirectionIdle

    };

    Page(Configuration &c);
    virtual ~Page();
    virtual void onNewItemSelected(Item *);
    bool pushCollection(CollectionInfo *collection);
    bool popCollection();
    void nextPlaylist();
    void pushMenu(ScrollingList *s);
    bool isMenusFull();
    void setLoadSound(Sound *chunk);
    void setUnloadSound(Sound *chunk);
    void setHighlightSound(Sound *chunk);
    void setSelectSound(Sound *chunk);
    bool addComponent(Component *c);
    void pageScroll(ScrollDirection direction);
    void letterScroll(ScrollDirection direction);
    void start();
    void startComponents();
    void stop();
    void setScrolling(ScrollDirection direction);
    bool isHorizontalScroll();
    unsigned int getMenuDepth();
    Item *getSelectedItem();
    void removeSelectedItem();
    void setScrollOffsetIndex(unsigned int i);
    unsigned int getScrollOffsetIndex();
    bool isIdle();
    bool isMenuIdle();
    bool isHidden();
    void setStatusTextComponent(Text *t);
    void update(float dt);
    void draw();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void launchEnter();
    void launchExit();
    std::string getCollectionName();
    void setMinShowTime(float value);
    float getMinShowTime();

private:
    void highlight();
    void playlistChange();
    std::string collectionName_;
    Configuration &config_;

    struct MenuInfo_S
    {
        CollectionInfo *collection;
        ScrollingList *menu;
        CollectionInfo::Playlists_T::iterator playlist; 
        bool queueDelete;
    };

    typedef std::vector<ScrollingList *> MenuVector_T;
    typedef std::list<MenuInfo_S> CollectionVector_T;

    ScrollingList *activeMenu_;
    unsigned int menuDepth_;
    MenuVector_T menus_;
    CollectionVector_T collections_;

    static const unsigned int NUM_LAYERS = 8;
    std::vector<Component *> LayerComponents[NUM_LAYERS];
    std::list<ScrollingList *> deleteMenuList_;
    std::list<CollectionInfo *> deleteCollectionList_;

    bool scrollActive_;

    Item *selectedItem_;
    Text *textStatusComponent_;
    bool selectedItemChanged_;
    bool playlistChanged_;
    Sound *loadSoundChunk_;
    Sound *unloadSoundChunk_;
    Sound *highlightSoundChunk_;
    Sound *selectSoundChunk_;
    float minShowTime_;
    float elapsedTime_;
    CollectionInfo::Playlists_T::iterator playlist_;


};
