/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once
#include "Component.h"
#include "../../Video/IVideo.h"
#include "../../Collection/Item.h"
#include <SDL2/SDL.h>
#include <string>

class Image;

//todo: this class should aggregate Image, Text, and Video component classes
class ReloadableMedia : public Component
{
public:
    ReloadableMedia(Configuration &config, std::string type, bool isVideo, float scaleX, float scaleY);
    virtual ~ReloadableMedia();
    void Update(float dt);
    void Draw();
    void FreeGraphicsMemory();
    void AllocateGraphicsMemory();
    void LaunchEnter();
    void LaunchExit();

private:
    void ReloadTexture();
    Configuration &Config;
    Component *LoadedComponent;
    bool ReloadRequested;
    bool FirstLoad;
    IVideo *VideoInst;

    bool IsVideo;
    std::string Type;
    float ScaleX;
    float ScaleY;
};
