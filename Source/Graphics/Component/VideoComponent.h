/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once
#include "Component.h"
#include "Image.h"
#include "../../Collection/Item.h"
#include "../../Video/IVideo.h"
#include <SDL2/SDL.h>
#include <string>

class VideoComponent : public Component
{
public:
	VideoComponent(IVideo *videoInst, std::string videoFile, float scaleX, float scaleY);
	virtual ~VideoComponent();
	void Update(float dt);
	void Draw();
   void FreeGraphicsMemory();
   void AllocateGraphicsMemory();
   void LaunchEnter() {FreeGraphicsMemory(); }
   void LaunchExit() { AllocateGraphicsMemory(); }

private:
   SDL_Texture *VideoTexture;
   std::string VideoFile;
   std::string Name;
   IVideo *VideoInst;
	float ScaleX;
	float ScaleY;
	bool IsPlaying;
};
