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

class Page
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
    void DeInitialize();
    virtual void onNewItemSelected();
    bool pushCollection(CollectionInfo *collection);
    bool popCollection();
    void enterMenu();
    void exitMenu();
    std::string getPlaylistName();
    void nextPlaylist();
    void selectPlaylist(std::string playlist);
    void pushMenu(ScrollingList *s);
    bool isMenusFull();
    void setLoadSound(Sound *chunk);
    void setUnloadSound(Sound *chunk);
    void setHighlightSound(Sound *chunk);
    void setSelectSound(Sound *chunk);
    bool addComponent(Component *c);
    void pageScroll(ScrollDirection direction);
    void letterScroll(ScrollDirection direction);
    unsigned int getCollectionSize();
    unsigned int getSelectedIndex();
    void selectRandom();
    void start();
    void stop();
    void setScrolling(ScrollDirection direction);
    bool isHorizontalScroll();
    unsigned int getMenuDepth();
    Item *getSelectedItem();
    Item *getSelectedItem(int offset);
    void removeSelectedItem();
    void setScrollOffsetIndex(unsigned int i);
    unsigned int getScrollOffsetIndex();
    bool isIdle();
    bool isGraphicsIdle();
    bool isMenuIdle();
    void setStatusTextComponent(Text *t);
    void update(float dt);
    void cleanup();
    void draw();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void launchEnter();
    void launchExit();
    std::string getCollectionName();
    void setMinShowTime(float value);
    float getMinShowTime();
    void menuScroll();
    void highlightEnter();
    void highlightExit();
    void addPlaylist();
    void removePlaylist();
    void reallocateMenuSpritePoints();
    bool isMenuScrolling();
    bool isPlaying();

private:
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
    CollectionVector_T deleteCollections_;

    static const unsigned int NUM_LAYERS = 20;
    std::vector<Component *> LayerComponents;
    std::list<ScrollingList *> deleteMenuList_;
    std::list<CollectionInfo *> deleteCollectionList_;

    bool scrollActive_;

    Item *selectedItem_;
    Text *textStatusComponent_;
    Sound *loadSoundChunk_;
    Sound *unloadSoundChunk_;
    Sound *highlightSoundChunk_;
    Sound *selectSoundChunk_;
    float minShowTime_;
    float elapsedTime_;
    CollectionInfo::Playlists_T::iterator playlist_;


};
