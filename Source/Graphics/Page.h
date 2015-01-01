/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "MenuNotifierInterface.h"

#include <vector>
#include <string>

class Component;
class ScrollingList;
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

   Page(std::string collectionName);
   virtual ~Page();
   virtual void OnNewItemSelected(Item *);
   void SetItems(std::vector<Item *> *items);
   void SetMenu(ScrollingList *s);
   void SetLoadSound(Sound *chunk) { LoadSoundChunk = chunk; }
   void SetUnloadSound(Sound *chunk) { UnloadSoundChunk = chunk; }
   void SetHighlightSound(Sound *chunk) { HighlightSoundChunk = chunk; }
   void SetSelectSound(Sound *chunk) { SelectSoundChunk = chunk; }
   bool AddComponent(Component *c);
   void PageScroll(ScrollDirection direction);
   void Start();
   void Stop();
   void SetScrolling(ScrollDirection direction);
   Item *GetSelectedItem();
   Item *GetPendingSelectedItem();
   void RemoveSelectedItem();
   bool IsIdle();
   bool IsHidden();
   void Update(float dt);
   void Draw();
   void FreeGraphicsMemory();
   void AllocateGraphicsMemory();
   void LaunchEnter();
   void LaunchExit();
   const std::string& GetCollectionName() const;

private:
   void Highlight();
   std::string CollectionName;
   ScrollingList *Menu;
   static const unsigned int NUM_LAYERS = 8;
   std::vector<Component *> LayerComponents[NUM_LAYERS];
   std::vector<Item *> *Items;
   bool ScrollActive;
   Item *SelectedItem;
   bool SelectedItemChanged;
   Sound *LoadSoundChunk;
   Sound *UnloadSoundChunk;
   Sound *HighlightSoundChunk;
   Sound *SelectSoundChunk;
   bool HasSoundedWhenActive;
   bool FirstSoundPlayed;

};
