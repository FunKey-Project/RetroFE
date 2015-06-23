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
#include <sstream>

Page::Page(Configuration &config)
    : Config(config)
    , ActiveMenu(NULL)
    , MenuDepth(0)
    , Items(NULL)
    , ScrollActive(false)
    , SelectedItem(NULL)
    , TextStatusComponent(NULL)
    , SelectedItemChanged(false)
    , LoadSoundChunk(NULL)
    , UnloadSoundChunk(NULL)
    , HighlightSoundChunk(NULL)
    , SelectSoundChunk(NULL)
    , MinShowTime(0)
{
}

Page::~Page()
{
    MenuVector_T::iterator it = Menus.begin();
    while(it != Menus.end())
    {
        ScrollingList *menu = *it;
        menu->RemoveComponentForNotifications(this);
        Menus.erase(it);
        delete menu;
        it = Menus.begin();
    }

    for(unsigned int i = 0; i < sizeof(LayerComponents)/sizeof(LayerComponents[0]); ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            delete *it;
        }

        LayerComponents[i].clear();
    }


    if(LoadSoundChunk)
    {
        delete LoadSoundChunk;
        LoadSoundChunk = NULL;
    }

    if(UnloadSoundChunk)
    {
        delete UnloadSoundChunk;
        UnloadSoundChunk = NULL;
    }


    if(HighlightSoundChunk)
    {
        delete HighlightSoundChunk;
        HighlightSoundChunk = NULL;
    }

    if(SelectSoundChunk)
    {
        delete SelectSoundChunk;
        SelectSoundChunk = NULL;
    }
}


bool Page::IsMenusFull()
{
  return (MenuDepth > Menus.size());
}

void Page::SetLoadSound(Sound *chunk)
{
  LoadSoundChunk = chunk;
}
void Page::SetUnloadSound(Sound *chunk)
{
  UnloadSoundChunk = chunk;
}
void Page::SetHighlightSound(Sound *chunk)
{
  HighlightSoundChunk = chunk;
}
void Page::SetSelectSound(Sound *chunk)
{
  SelectSoundChunk = chunk;
}
void Page::OnNewItemSelected(Item *item)
{
    SelectedItem = item;
    SelectedItemChanged = true;
}

void Page::PushMenu(ScrollingList *s)
{
    Menus.push_back(s);

    if(s)
    {
        s->AddComponentForNotifications(this);
    }
}

unsigned int Page::GetMenuDepth()
{
    return MenuDepth;
}

void Page::SetStatusTextComponent(Text *t)
{
    TextStatusComponent = t;
}

bool Page::AddComponent(Component *c)
{
    bool retVal = false;

    unsigned int layer = c->BaseViewInfo.GetLayer();


    if(layer < NUM_LAYERS)
    {
        LayerComponents[layer].push_back(c);

        retVal = true;
    }
    else
    {
        std::stringstream ss;
        ss << "Component layer too large Layer: " << layer;
        Logger::Write(Logger::ZONE_ERROR, "Page", ss.str());
    }

    return retVal;
}

bool Page::IsMenuIdle()
{
    bool idle = true;

    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;

        if(!menu->IsIdle())
        {
            idle = false;
            break;
        }
    }
    return idle;
}

bool Page::IsIdle()
{
    bool idle = IsMenuIdle();
    
    for(unsigned int i = 0; i < NUM_LAYERS && idle; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end() && idle; ++it)
        {
            idle = (*it)->IsIdle();
        }
    }

    return idle;
}


bool Page::IsHidden()
{
    bool hidden = true;

    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;

        if(!menu->IsHidden())
        {
            hidden = false;
            break;
        }
    }


    for(unsigned int i = 0; hidden && i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); hidden && it != LayerComponents[i].end(); ++it)
        {
            hidden = (*it)->IsHidden();
        }
    }

    return hidden;
}

void Page::Start()
{
    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->TriggerEnterEvent();
    }

    if(LoadSoundChunk)
    {
        LoadSoundChunk->Play();
    }

    StartComponents();
}


void Page::StartComponents()
{
    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->TriggerEnterEvent();
        }
    }
}

void Page::Stop()
{
    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->TriggerExitEvent();
    }

    if(UnloadSoundChunk)
    {
        UnloadSoundChunk->Play();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->TriggerExitEvent();
        }
    }
}


Item *Page::GetSelectedItem()
{
    return SelectedItem;
}

void Page::RemoveSelectedItem()
{
    /*
    //todo: change method to RemoveItem() and pass in SelectedItem
    if(Menu)
    {
        Menu->RemoveSelectedItem();
    }
    */
    SelectedItem = NULL;

}

void Page::SetScrollOffsetIndex(unsigned int i)
{
    if(!ActiveMenu) return;

    ActiveMenu->SetScrollOffsetIndex(i);
}

unsigned int Page::GetScrollOffsetIndex()
{
    if(!ActiveMenu) return -1;

    return ActiveMenu->GetScrollOffsetIndex();
}

void Page::SetMinShowTime(float value)
{
    MinShowTime = value;
}

float Page::GetMinShowTime()
{
    return MinShowTime;
}

void Page::Highlight()
{
    Item *item = SelectedItem;

    if(item)
    {
        if(ActiveMenu)
        {
            ActiveMenu->TriggerHighlightEvent(item);
            ActiveMenu->SetScrollActive(ScrollActive);
        }

        for(unsigned int i = 0; i < NUM_LAYERS; ++i)
        {
            for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
            {
                (*it)->TriggerHighlightEvent(item);
                (*it)->SetScrollActive(ScrollActive);
            }
        }
    }
}


void Page::SetScrolling(ScrollDirection direction)
{
    ScrollingList::ScrollDirection menuDirection;
    bool prevScrollActive = ScrollActive;

    switch(direction)
    {
    case ScrollDirectionForward:
        menuDirection = ScrollingList::ScrollDirectionForward;
        ScrollActive = true;
        break;
    case ScrollDirectionBack:
        menuDirection = ScrollingList::ScrollDirectionBack;
        ScrollActive = true;
        break;
    case ScrollDirectionIdle:
    default:
        menuDirection = ScrollingList::ScrollDirectionIdle;
        ScrollActive = false;
        break;
    }

    if(!prevScrollActive && ScrollActive && HighlightSoundChunk)
    {
        HighlightSoundChunk->Play();
    }


    ActiveMenu->SetScrollDirection(menuDirection);
}

bool Page::IsHorizontalScroll()
{
   if(!ActiveMenu) { return false; }

   return ActiveMenu->IsHorizontalScroll();
}

void Page::PageScroll(ScrollDirection direction)
{
    if(ActiveMenu)
    {
        if(direction == ScrollDirectionForward)
        {
            ActiveMenu->PageDown();
        }
        if(direction == ScrollDirectionBack)
        {
            ActiveMenu->PageUp();
        }
    }
}

void Page::LetterScroll(ScrollDirection direction)
{
    if(ActiveMenu)
    {
        if(direction == ScrollDirectionForward)
        {
            ActiveMenu->LetterDown();
        }
        if(direction == ScrollDirectionBack)
        {
            ActiveMenu->LetterUp();
        }
    }
}


bool Page::PushCollection(CollectionInfo *collection)
{
    Collections.push_back(collection);
    std::vector<ComponentItemBinding *> *sprites = ComponentItemBindingBuilder::BuildCollectionItems(&collection->Items);

    int menuExitIndex = -1;
    int menuEnterIndex = -1;

    if(ActiveMenu)
    {
        ActiveMenu->TriggerMenuExitEvent();
    }

    if(MenuDepth > 0)
    {
        menuExitIndex = MenuDepth - 1;
    }

    if(Menus.size() >= MenuDepth && ActiveMenu)
    {
        ScrollingList *newList = new ScrollingList(*ActiveMenu);
        newList->ForceIdle();
        PushMenu(newList);
    }

    ActiveMenu = Menus[MenuDepth];
    ActiveMenu->SetCollectionName(collection->Name);
    ActiveMenu->DestroyItems();
    ActiveMenu->SetItems(sprites);
    ActiveMenu->TriggerMenuEnterEvent();

    if(MenuDepth < Menus.size())
    {
        menuEnterIndex = MenuDepth;
        MenuDepth++;
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->SetCollectionName(collection->Name);
            if(menuEnterIndex >= 0)
            {
                (*it)->TriggerMenuEnterEvent(menuEnterIndex);
            }

            if(menuExitIndex >= 0)
            {
                (*it)->TriggerMenuExitEvent(menuExitIndex);
            }
        }
    }

    return true;
}

bool Page::PopCollection()
{
    int menuExitIndex = -1;
    int menuEnterIndex = -1;
    CollectionInfo *collection = NULL;
    if(MenuDepth <= 1)
    {
        return false;
    }
    if(Collections.size() <= 1)
    {
        return false;
    }

    Collections.pop_back();
    collection = Collections.back();

    if(ActiveMenu)
    {
        ActiveMenu->TriggerMenuExitEvent();
    }

    MenuDepth--;
    menuExitIndex = MenuDepth;
    menuEnterIndex = menuExitIndex - 1;
    ActiveMenu = Menus[MenuDepth - 1];
    if(ActiveMenu)
    {
        ActiveMenu->TriggerMenuEnterEvent();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->SetCollectionName(collection->Name);

            if(menuEnterIndex >= 0)
            {
                (*it)->TriggerMenuEnterEvent(menuEnterIndex);
            }

            if(menuExitIndex >= 0)
            {
                (*it)->TriggerMenuExitEvent(menuExitIndex);
            }
        }
    }

    return true;
}


void Page::Update(float dt)
{
    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;

        menu->Update(dt);
    }

    if(SelectedItemChanged && !ScrollActive)
    {
        Highlight();
        SelectedItemChanged = false;
    }

    if(TextStatusComponent)
    {
    	std::string status;
    	Config.SetProperty("status", status);
        TextStatusComponent->SetText(status);
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->Update(dt);
        }
    }
}

void Page::Draw()
{
    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->Draw();
        }

        for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
        {
            ScrollingList *menu = *it;
            menu->Draw(i);
        }
    }

}

std::string Page::GetCollectionName()
{
    CollectionInfo *info = Collections.back();

    if(info)
    {
        return info->Name;
    }

    return "";
}

void Page::FreeGraphicsMemory()
{
    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->FreeGraphicsMemory();
    }

    if(LoadSoundChunk) LoadSoundChunk->Free();
    if(UnloadSoundChunk) UnloadSoundChunk->Free();
    if(HighlightSoundChunk) HighlightSoundChunk->Free();
    if(SelectSoundChunk) SelectSoundChunk->Free();

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->FreeGraphicsMemory();
        }
    }
}

void Page::AllocateGraphicsMemory()
{
    Logger::Write(Logger::ZONE_DEBUG, "Page", "Allocating graphics memory");

    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;

        menu->AllocateGraphicsMemory();
    }

    if(LoadSoundChunk) LoadSoundChunk->Allocate();
    if(UnloadSoundChunk) UnloadSoundChunk->Allocate();
    if(HighlightSoundChunk) HighlightSoundChunk->Allocate();
    if(SelectSoundChunk) SelectSoundChunk->Allocate();

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->AllocateGraphicsMemory();
        }
    }
    Logger::Write(Logger::ZONE_DEBUG, "Page", "Allocate graphics memory complete");
}

void Page::LaunchEnter()
{
    if(ActiveMenu)
    {
        ActiveMenu->LaunchEnter();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->LaunchEnter();
        }
    }
}

void Page::LaunchExit()
{
    if(ActiveMenu)
    {
        ActiveMenu->LaunchExit();
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->LaunchExit();
        }
    }
}
