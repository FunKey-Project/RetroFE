/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <vector>

#include "../MenuNotifierInterface.h"
#include "../ViewInfo.h"
#include "../Animate/Tween.h"
#include "../../Collection/Item.h"

class Component
{
public:
	Component();
	virtual ~Component();
   virtual void FreeGraphicsMemory();
   virtual void AllocateGraphicsMemory();
   virtual void LaunchEnter() {}
   virtual void LaunchExit() {}
	void TriggerEnterEvent();
	void TriggerExitEvent();
	void TriggerHighlightEvent(Item *selectedItem);
	bool IsIdle();
	bool IsHidden();
	bool IsWaiting();
	typedef std::vector<std::vector<Tween *> *> TweenSets;

	void SetOnEnterTweens(TweenSets *tweens)
	{
		this->OnEnterTweens = tweens;
	}

	void SetOnExitTweens(TweenSets *tweens)
	{
		this->OnExitTweens = tweens;
	}

	void SetOnIdleTweens(TweenSets *tweens)
	{
		this->OnIdleTweens = tweens;
	}

	void SetOnHighlightEnterTweens(TweenSets *tweens)
	{
		this->OnHighlightEnterTweens = tweens;
	}

	void SetOnHighlightExitTweens(TweenSets *tweens)
	{
		this->OnHighlightExitTweens = tweens;
	}
	virtual void Update(float dt);

	virtual void Draw() = 0;

   ViewInfo *GetBaseViewInfo()
   {
      return &BaseViewInfo;
   }
   void UpdateBaseViewInfo(ViewInfo &info)
   {
     BaseViewInfo = info;
   }

   bool IsScrollActive() const
   {
      return ScrollActive;
   }

   void SetScrollActive(bool scrollActive)
   {
      ScrollActive = scrollActive;
   }



protected:
	Item *GetSelectedItem()
	{
		return SelectedItem;
	}
   enum AnimationState
   {
      IDLE,
      ENTER,
      HIGHLIGHT_EXIT,
      HIGHLIGHT_WAIT,
      HIGHLIGHT_ENTER,
      EXIT,
      HIDDEN
   };

   AnimationState CurrentAnimationState;
   bool EnterRequested;
   bool ExitRequested;
   bool NewItemSelected;
   bool HighlightExitComplete;
   bool NewItemSelectedSinceEnter;
private:
	bool Animate(bool loop);
	bool IsTweenSequencingComplete();
	void ResetTweenSequence(std::vector<ViewInfo *> *tweens);

	TweenSets *OnEnterTweens;
	TweenSets *OnExitTweens;
	TweenSets *OnIdleTweens;
	TweenSets *OnHighlightEnterTweens;
	TweenSets *OnHighlightExitTweens;

	TweenSets *CurrentTweens;
   unsigned int CurrentTweenIndex;

	bool CurrentTweenComplete;
	ViewInfo BaseViewInfo;

	float ElapsedTweenTime;
	Tween *TweenInst;
	Item *SelectedItem;
   bool ScrollActive;
};
