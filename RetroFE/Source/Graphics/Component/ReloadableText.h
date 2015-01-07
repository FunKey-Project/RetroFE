/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once
#include "Component.h"
#include "Text.h"
#include "../Font.h"
#include "../../Collection/Item.h"
#include <SDL2/SDL.h>
#include <string>

class ReloadableText : public Component
{
public:
    ReloadableText(std::string type, Font *font, SDL_Color color, std::string layoutKey, std::string collectionName, float scaleX, float scaleY);
    virtual ~ReloadableText();
    void Update(float dt);
    void Draw();
    void FreeGraphicsMemory();
    void AllocateGraphicsMemory();
    void LaunchEnter();
    void LaunchExit();

private:
    enum TextType
    {
        TextTypeUnknown = 0,
        TextTypeNumberButtons,
        TextTypeNumberPlayers,
        TextTypeYear,
        TextTypeTitle,
        TextTypeManufacturer,
    };

    void ReloadTexture();

    Text *ImageInst;
    TextType Type;
    std::string LayoutKey;
    std::string Collection;
    bool ReloadRequested;
    bool FirstLoad;
    Font *FontInst;
    SDL_Color FontColor;

    float ScaleX;
    float ScaleY;
};
