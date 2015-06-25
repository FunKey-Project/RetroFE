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
    : textData_(text)
    , fontInst_(font)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
{
    allocateGraphicsMemory();
}

Text::~Text()
{
    freeGraphicsMemory();
}

void Text::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();
}

void Text::allocateGraphicsMemory()
{
    //todo: make the font blend color a parameter that is passed in
    Component::allocateGraphicsMemory();
}

void Text::setText(std::string text)
{
    textData_ = text;
}

void Text::draw()
{
    Component::draw();

    SDL_Texture *t = fontInst_->getTexture();

    float imageHeight = 0;
    float imageWidth = 0;

    // determine image width
    for(unsigned int i = 0; i < textData_.size(); ++i)
    {
        Font::GlyphInfo glyph;
        if(fontInst_->getRect(textData_[i], glyph))
        {
            if(glyph.minX < 0)
            {
                imageWidth += glyph.minX;
            }

            imageWidth += glyph.advance;
        }

    }

    imageHeight = (float)fontInst_->getHeight();
    float scale = (float)baseViewInfo.FontSize / (float)imageHeight;

    float oldWidth = baseViewInfo.Width;
    float oldHeight = baseViewInfo.Height;
    float oldImageWidth = baseViewInfo.ImageHeight;
    float oldImageHeight = baseViewInfo.ImageWidth;

    baseViewInfo.Width = imageWidth*scale;
    baseViewInfo.Height = baseViewInfo.FontSize;
    baseViewInfo.ImageWidth = imageWidth;
    baseViewInfo.ImageHeight = imageHeight;

    float xOrigin = baseViewInfo.XRelativeToOrigin();
    float yOrigin = baseViewInfo.YRelativeToOrigin();

    baseViewInfo.Width = oldWidth;
    baseViewInfo.Height = oldHeight;
    baseViewInfo.ImageWidth = oldImageWidth;
    baseViewInfo.ImageHeight = oldImageHeight;


    SDL_Rect rect;
    rect.x = static_cast<int>(xOrigin);

    for(unsigned int i = 0; i < textData_.size(); ++i)
    {
        Font::GlyphInfo glyph;

        if(fontInst_->getRect(textData_[i], glyph) && glyph.rect.h > 0)
        {
            SDL_Rect charRect = glyph.rect;
            float h = static_cast<float>(charRect.h * scale);
            float w = static_cast<float>(charRect.w * scale);
            rect.h = static_cast<int>(h);
            rect.w = static_cast<int>(w);
            rect.y = static_cast<int>(yOrigin);

            if(glyph.minX < 0)
            {
                rect.x += static_cast<int>((float)(glyph.minX) * scale);
            }
            if(fontInst_->getAscent() < glyph.maxY)
            {
                rect.y += static_cast<int>((fontInst_->getAscent() - glyph.maxY)*scale);
            }


            SDL::renderCopy(t, static_cast<char>(baseViewInfo.Alpha * 255), &charRect, &rect, baseViewInfo.Angle);

            rect.x += static_cast<int>(glyph.advance * scale);

            if((static_cast<float>(rect.x) - xOrigin) > baseViewInfo.MaxWidth)
            {
                break;
            }
        }
    }
}

