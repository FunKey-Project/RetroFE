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
#include "../Utility/Log.h"
#include "Component/ScrollingList.h"
#include "../Sound/Sound.h"
#include "ComponentItemBindingBuilder.h"
#include <sstream>

Page::Page(std::string collectionName)
    : CollectionName(collectionName)
    , Menu(NULL)
    , Items(NULL)
    , ScrollActive(false)
    , SelectedItem(NULL)
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
    if(Menu)
    {
        Menu->RemoveComponentForNotifications(this);
    }

    for(unsigned int i = 0; i < sizeof(LayerComponents)/sizeof(LayerComponents[0]); ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            delete *it;
        }

        LayerComponents[i].clear();
    }

    if(Menu)
    {
        delete Menu;
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

void Page::SetMenu(ScrollingList *s)
{
    // todo: delete the old menu
    Menu = s;
    Menu->AddComponentForNotifications(this);
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

    if(!Menu->IsIdle())
    {
        idle = false;
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
    bool hidden = Menu->IsHidden();

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
    Menu->TriggerEnterEvent();

    if(LoadSoundChunk)
    {
        LoadSoundChunk->Play();
    }

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
    Menu->TriggerExitEvent();

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
    //todo: change method to RemoveItem() and pass in SelectedItem
    Menu->RemoveSelectedItem();
    SelectedItem = NULL;

}


void Page::Highlight()
{
    Item *item = SelectedItem;

    if(item)
    {
        Menu->TriggerHighlightEvent(item);
        Menu->SetScrollActive(ScrollActive);

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

    Menu->SetScrollDirection(menuDirection);
}

void Page::PageScroll(ScrollDirection direction)
{
    if(direction == ScrollDirectionForward)
    {
        Menu->PageDown();
    }
    if(direction == ScrollDirectionBack)
    {
        Menu->PageUp();
    }
}


void Page::SetItems(std::vector<Item *> *items)
{
    std::vector<ComponentItemBinding *> *sprites = ComponentItemBindingBuilder::BuildCollectionItems(items);

    Menu->SetItems(sprites);
}



void Page::Update(float dt)
{
    Menu->Update(dt);

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

        Menu->Draw(i);
    }
}

const std::string& Page::GetCollectionName() const
{
    return CollectionName;
}

void Page::FreeGraphicsMemory()
{
    Logger::Write(Logger::ZONE_DEBUG, "Page", "Free");
    Menu->FreeGraphicsMemory();

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
    Menu->AllocateGraphicsMemory();

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
    Menu->LaunchEnter();

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
    Menu->LaunchExit();

    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents[i].begin(); it != LayerComponents[i].end(); ++it)
        {
            (*it)->LaunchExit();
        }
    }
}

