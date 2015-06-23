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

#include "Text.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../Font.h"
#include <sstream>

Text::Text(std::string text, Font *font, float scaleX, float scaleY)
    : TextData(text)
    , FontInst(font)
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

void Text::SetText(std::string text)
{
    TextData = text;
}

void Text::Draw()
{
    Component::Draw();

    SDL_Texture *t = FontInst->GetTexture();

    float imageHeight = 0;
    float imageWidth = 0;

    // determine image width
    for(unsigned int i = 0; i < TextData.size(); ++i)
    {
        Font::GlyphInfo glyph;
        if(FontInst->GetRect(TextData[i], glyph))
        {
            if(glyph.MinX < 0)
            {
                imageWidth += glyph.MinX;
            }

            imageWidth += glyph.Advance;
        }

    }

    imageHeight = (float)FontInst->GetHeight();
    float scale = (float)BaseViewInfo.FontSize / (float)imageHeight;

    float oldWidth = BaseViewInfo.Width;
    float oldHeight = BaseViewInfo.Height;
    float oldImageWidth = BaseViewInfo.ImageHeight;
    float oldImageHeight = BaseViewInfo.ImageWidth;

    BaseViewInfo.Width = imageWidth*scale;
    BaseViewInfo.Height = BaseViewInfo.FontSize;
    BaseViewInfo.ImageWidth = imageWidth;
    BaseViewInfo.ImageHeight = imageHeight;

    float xOrigin = BaseViewInfo.XRelativeToOrigin();
    float yOrigin = BaseViewInfo.YRelativeToOrigin();

    BaseViewInfo.Width = oldWidth;
    BaseViewInfo.Height = oldHeight;
    BaseViewInfo.ImageWidth = oldImageWidth;
    BaseViewInfo.ImageHeight = oldImageHeight;


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

            if(glyph.MinX < 0)
            {
                rect.x += static_cast<int>((float)(glyph.MinX) * scale);
            }
            if(FontInst->GetAscent() < glyph.MaxY)
            {
                rect.y += static_cast<int>((FontInst->GetAscent() - glyph.MaxY)*scale);
            }


            SDL::RenderCopy(t, static_cast<char>(BaseViewInfo.Alpha * 255), &charRect, &rect, BaseViewInfo.Angle);

            rect.x += static_cast<int>(glyph.Advance * scale);

            if((static_cast<float>(rect.x) - xOrigin) > BaseViewInfo.MaxWidth)
            {
                break;
            }
        }
    }
}

