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
    , HasSoundedWhenActive(false)
    , FirstSoundPlayed(false)
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


void Page::SetStatusTextComponent(Text *t)
{
    TextStatusComponent = t;
}

bool Page::AddComponent(Component *c)
{
    bool retVal = false;

    unsigned int layer = c->GetBaseViewInfo()->GetLayer();


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

bool Page::IsIdle()
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
        Logger::Write(Logger::ZONE_DEBUG, "Page", "Triggering enter event");
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

    if(ActiveMenu)
    {
        ActiveMenu->SetScrollDirection(menuDirection);
    }
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

bool Page::PushCollection(CollectionInfo *collection)
{
    Collections.push_back(collection);
    std::vector<ComponentItemBinding *> *sprites = ComponentItemBindingBuilder::BuildCollectionItems(collection->GetItems());

    if(ActiveMenu)
    {
        Logger::Write(Logger::ZONE_INFO, "Page", "Trigger exit event, expect focus immediate");
        ActiveMenu->TriggerMenuExitEvent();
    }
    
    ActiveMenu = Menus[MenuDepth];

    ActiveMenu->SetItems(sprites);
    ActiveMenu->SetCollectionName(collection->GetName());
    Logger::Write(Logger::ZONE_INFO, "Page", "Trigger enter event, expect focus immediate");
    ActiveMenu->TriggerMenuEnterEvent();

    if(MenuDepth < Menus.size())
    {
        MenuDepth++;
    }

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->SetCollectionName(collection->GetName());
        }
    }
    Logger::Write(Logger::ZONE_INFO, "Page", "PUSH COMPLETE, expect updates");

    return true;
}

bool Page::PopCollection()
{
    if(MenuDepth > 1)
    {
        if(Collections.size() > 1)
        {
            Collections.pop_back();
        }

        if(ActiveMenu)
        {
            ActiveMenu->TriggerMenuExitEvent();
        }

        MenuDepth--;

        ActiveMenu = Menus[MenuDepth - 1];
        if(ActiveMenu)
        {
            ActiveMenu->TriggerMenuEnterEvent();
        }

        return true;
    }
    return false;
}


void Page::Update(float dt)
{
    for(MenuVector_T::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
        ScrollingList *menu = *it;

        menu->Update(dt);
    }

    if(SelectedItemChanged && !HasSoundedWhenActive && HighlightSoundChunk)
    {
        // skip the first sound being played (as it is part of the on-enter)
        if(FirstSoundPlayed)
        {
            HighlightSoundChunk->Play();
            HasSoundedWhenActive = true;
        }
        FirstSoundPlayed = true;
    }

    if(SelectedItemChanged && !ScrollActive)
    {
        Highlight();
        SelectedItemChanged = false;
        HasSoundedWhenActive = false;
    }

    if(TextStatusComponent)
    {
        TextStatusComponent->SetText(Config.GetStatus());
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
        return info->GetName();
    }

    return "";
}

void Page::FreeGraphicsMemory()
{
    Logger::Write(Logger::ZONE_DEBUG, "Page", "Free");

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
    FirstSoundPlayed = false;
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

