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
#pragma once

#include "Animate/TweenTypes.h"
#include <string>
#include <map>

class ViewInfo
{
public:

    ViewInfo();
    virtual ~ViewInfo();

    float GetXRelativeToOrigin() const;
    float GetYRelativeToOrigin() const;

    float GetHeight() const;
    float GetWidth() const;

    float GetAngle() const;
    void SetAngle(float angle);
    float GetImageHeight() const;
    void SetImageHeight(float imageheight);
    float GetImageWidth() const;
    void SetImageWidth(float imagewidth);
    unsigned int GetLayer() const;
    void SetLayer(unsigned int layer);
    float GetMaxHeight() const;
    void SetMaxHeight(float maxheight);
    float GetMaxWidth() const;
    void SetMaxWidth(float maxwidth);
    float GetMinHeight() const;
    void SetMinHeight(float minheight);
    float GetMinWidth() const;
    void SetMinWidth(float minwidth);
    float GetAlpha() const;
    void SetAlpha(float alpha);
    float GetX() const;
    void SetX(float x);
    float GetXOffset() const;
    void SetXOffset(float offset);
    float GetXOrigin() const;
    void SetXOrigin(float origin);
    float GetY() const;
    void SetY(float y);
    float GetYOffset() const;
    void SetYOffset(float offset);
    float GetYOrigin() const;
    void SetYOrigin(float origin);
    float GetRawYOrigin();
    float GetRawXOrigin();
    float GetRawWidth();
    float GetRawHeight();

    float GetBackgroundRed();
    void SetBackgroundRed(float value);
    float GetBackgroundGreen();
    void SetBackgroundGreen(float value);
    float GetBackgroundBlue();
    void SetBackgroundBlue(float value);
    float GetBackgroundAlpha();
    void SetBackgroundAlpha(float value);

    void SetHeight(float height);
    void SetWidth(float width);
    float GetFontSize() const;
    void SetFontSize(float fontSize);

    static const int AlignCenter = -1;
    static const int AlignLeft = -2;
    static const int AlignTop = -3;
    static const int AlignRight = -4;
    static const int AlignBottom = -5;

private:
    float GetAbsoluteHeight() const;
    float GetAbsoluteWidth() const;
    float X;
    float Y;
    float XOrigin;
    float YOrigin;
    float XOffset;
    float YOffset;
    float Width;
    float MinWidth;
    float MaxWidth;
    float Height;
    float MinHeight;
    float MaxHeight;
    float ImageWidth;
    float ImageHeight;
    float FontSize;
    float Angle;
    float Alpha;
    unsigned int Layer;
    float HorizontalScale;
    float VerticalScale;
    float BackgroundRed;
    float BackgroundGreen;
    float BackgroundBlue;
    float BackgroundAlpha;
};
