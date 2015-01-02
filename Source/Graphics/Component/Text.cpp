/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Text.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../Font.h"
#include <sstream>

Text::Text(std::string text, Font *font, SDL_Color fontColor, float scaleX, float scaleY)
    : TextData(text)
    , FontInst(font)
    , FontColor(fontColor)
    , ScaleX(scaleX)
    , ScaleY(scaleY)
{
    AllocateGraphicsMemory();
}

Text::~Text()
{
    FreeGraphicsMemory();
}


void Text::FreeGraphicsMemory()
{
    Component::FreeGraphicsMemory();
}

void Text::AllocateGraphicsMemory()
{
    //todo: make the font blend color a parameter that is passed in
    Component::AllocateGraphicsMemory();
}

void Text::Draw()
{
    Component::Draw();

    SDL_Texture *t = FontInst->GetTexture();

    ViewInfo *info = GetBaseViewInfo();
    float imageHeight = 0;
    float imageWidth = 0;

    // determine image width
    for(unsigned int i = 0; i < TextData.size(); ++i)
    {
        Font::GlyphInfo glyph;
        if(FontInst->GetRect(TextData[i], glyph))
        {
            imageWidth += glyph.Advance;
            imageHeight = (imageHeight >= glyph.Rect.h) ? imageHeight : glyph.Rect.h;
        }
    }

    float scale = (float)info->GetFontSize() / (float)imageHeight;


    float width = info->GetRawWidth();
    float height = info->GetRawHeight();

    info->SetWidth(imageWidth*scale);
    info->SetHeight(imageHeight*scale);

    float xOrigin = info->GetXRelativeToOrigin();
    float yOrigin = info->GetYRelativeToOrigin();

    info->SetWidth(width);
    info->SetHeight(height);


    SDL_Rect rect;
    rect.x = static_cast<int>(xOrigin);

    for(unsigned int i = 0; i < TextData.size(); ++i)
    {
        Font::GlyphInfo glyph;

        if(FontInst->GetRect(TextData[i], glyph) && glyph.Rect.h > 0)
        {
            SDL_Rect charRect = glyph.Rect;
            float h = static_cast<float>(charRect.h * scale);
            float w = static_cast<float>(charRect.w * scale);
            rect.h = static_cast<int>(h);
            rect.w = static_cast<int>(w);
            rect.y = static_cast<int>(yOrigin);

            SDL_LockMutex(SDL::GetMutex());
            SDL_SetTextureColorMod(t, FontColor.r, FontColor.g, FontColor.b);
            SDL_UnlockMutex(SDL::GetMutex());

            SDL::RenderCopy(t, static_cast<char>(info->GetAlpha() * 255), &charRect, &rect, info->GetAngle());
            rect.x += static_cast<int>(glyph.Advance * scale);
        }
    }
}

