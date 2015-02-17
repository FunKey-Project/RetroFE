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

#include <map>
#include <string>
#include <list>
#include <vector>

class CollectionInfo;
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
    virtual void OnNewItemSelected(Item *);
    bool PushCollection(CollectionInfo *collection);
    bool IsMenusFull()
    {
        return (MenuDepth > Menus.size());
    }
    bool PopCollection();
    void PushMenu(ScrollingList *s);

    void SetLoadSound(Sound *chunk)
    {
        LoadSoundChunk = chunk;
    }
    void SetUnloadSound(Sound *chunk)
    {
        UnloadSoundChunk = chunk;
    }
    void SetHighlightSound(Sound *chunk)
    {
        HighlightSoundChunk = chunk;
    }
    void SetSelectSound(Sound *chunk)
    {
        SelectSoundChunk = chunk;
    }

    bool AddComponent(Component *c);
    void PageScroll(ScrollDirection direction);
    void Start();
    void StartComponents();
    void Stop();
    void SetScrolling(ScrollDirection direction);
    unsigned int GetMenuDepth()
    {
        return MenuDepth;
    }
    Item *GetSelectedItem();
    Item *GetPendingSelectedItem();
    void RemoveSelectedItem();
    bool IsIdle();
    bool IsHidden();
    void SetStatusTextComponent(Text *t);
    void Update(float dt);
    void Draw();
    void FreeGraphicsMemory();
    void AllocateGraphicsMemory();
    void LaunchEnter();
    void LaunchExit();
    std::string GetCollectionName();

private:
    void Highlight();
    std::string CollectionName;
    Configuration &Config;
    typedef std::vector<ScrollingList *> MenuVector_T;
    typedef std::vector<CollectionInfo *> CollectionInfo_T;

    ScrollingList *ActiveMenu;
    unsigned int MenuDepth;
    MenuVector_T Menus;
    CollectionInfo_T Collections;

    static const unsigned int NUM_LAYERS = 8;
    std::vector<Component *> LayerComponents[NUM_LAYERS];
    std::vector<Item *> *Items;
    bool ScrollActive;

    Item *SelectedItem;
    Text *TextStatusComponent;
    bool SelectedItemChanged;
    Sound *LoadSoundChunk;
    Sound *UnloadSoundChunk;
    Sound *HighlightSoundChunk;
    Sound *SelectSoundChunk;
    bool HasSoundedWhenActive;
    bool FirstSoundPlayed;


};
