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

#include "ViewInfo.h"
#include "../Database/Configuration.h"
#include "Animate/TweenTypes.h"
#include <cfloat>

ViewInfo::ViewInfo()
    : X(0)
    , Y(0)
    , XOrigin(0)
    , YOrigin(0)
    , XOffset(0)
    , YOffset(0)
    , Width(-1)
    , MinWidth(0)
    , MaxWidth(FLT_MAX)
    , Height(-1)
    , MinHeight(0)
    , MaxHeight(FLT_MAX)
    , ImageWidth(0)
    , ImageHeight(0)
    , FontSize(-1)
    , Angle(0)
    , Alpha(1)
    , Layer(0)
    , BackgroundRed(0)
    , BackgroundGreen(0)
    , BackgroundBlue(0)
    , BackgroundAlpha(0)
{
}


ViewInfo::~ViewInfo()
{
}

float ViewInfo::GetXRelativeToOrigin() const
{
    return X + XOffset - XOrigin*GetWidth();
}

float ViewInfo::GetYRelativeToOrigin() const
{
    return Y + YOffset - YOrigin*GetHeight();
}

float ViewInfo::GetHeight() const
{
    float height = GetAbsoluteHeight();
    float width = GetAbsoluteWidth();

    if (height < MinHeight || width < MinWidth)
    {
        float scaleH = MinHeight / height;
        float scaleW = MinWidth / width;

        if(width >= MinWidth && height < MinHeight) 
        {
            height = MinHeight;
        }
        else if(width < MinWidth && height >= MinHeight) 
        {
            height = scaleW * height;
        }
        else
        {
            height = (scaleH > scaleW) ? (MinHeight) : (height * scaleW);
        }
    }
    if (width > MaxWidth || height > MaxHeight)
    {
        float scaleH = MaxHeight / height;
        float scaleW = MaxWidth / width;

        if(width <= MaxWidth && height > MaxHeight)
        {
            height = MaxHeight;
        }
        if(width > MaxWidth && height <= MaxHeight)
        {
            height = scaleW * height;
        }
        else
        {
            height = (scaleH < scaleW) ? (MaxHeight) : (height * scaleW);
        }
    }

    return height;
}

float ViewInfo::GetWidth() const
{
    float height = GetAbsoluteHeight();
    float width = GetAbsoluteWidth();

    if (height < MinHeight || width < MinWidth)
    {
        float scaleH = MinHeight / height;
        float scaleW = MinWidth / width;

        if(height >= MinHeight && width < MinWidth) 
        {
            width = MinWidth;
        }
        else if(height < MinHeight && width >= MinWidth) 
        {
            width = scaleH * width;
        }
        else
        {
            width = (scaleH > scaleW) ? (MinWidth) : (width * scaleH);
        }
    }
    if (width > MaxWidth || height > MaxHeight)
    {
        float scaleH = MaxHeight / height;
        float scaleW = MaxWidth / width;

        if(height <= MaxHeight && width > MaxWidth)
        {
            width = MaxWidth;
        }
        if(height > MaxHeight && width <= MaxWidth)
        {
            width = scaleH * width;
        }
        else
        {
            width = (scaleH > scaleW) ? (MaxWidth) : (width * scaleH);
        }
    }

    return width;
}

float ViewInfo::GetAbsoluteHeight() const
{
    if(Height == -1 && Width == -1)
    {
        return ImageHeight;
    }

    if (Height == -1 && ImageWidth != 0)
    {
        return ImageHeight * Width / ImageWidth;
    }

    return Height;
}

float ViewInfo::GetAbsoluteWidth() const
{
    if(Height == -1 && Width == -1)
    {
        return ImageWidth;
    }

    if (Width == -1 && ImageHeight != 0)
    {
        return ImageWidth * Height / ImageHeight;
    }

    return Width;
}


float ViewInfo::GetXOffset() const
{
    return XOffset;
}


float ViewInfo::GetXOrigin() const
{
    return XOrigin;
}


float ViewInfo::GetYOffset() const
{
    return YOffset;
}


float ViewInfo::GetYOrigin() const
{
    return YOrigin;
}

float ViewInfo::GetAngle() const
{
    return Angle;
}

void ViewInfo::SetAngle(float angle)
{
    Angle = angle;
}

float ViewInfo::GetImageHeight() const
{
    return ImageHeight;
}

void ViewInfo::SetImageHeight(float imageheight)
{
    ImageHeight = imageheight;
}

float ViewInfo::GetImageWidth() const
{
    return ImageWidth;
}

void ViewInfo::SetImageWidth(float imagewidth)
{
    ImageWidth = imagewidth;
}

unsigned int ViewInfo::GetLayer() const
{
    return Layer;
}

void ViewInfo::SetLayer(unsigned int layer)
{
    Layer = layer;
}

float ViewInfo::GetMaxHeight() const
{
    return MaxHeight;
}

void ViewInfo::SetMaxHeight(float maxheight)
{
    MaxHeight = maxheight;
}

float ViewInfo::GetMaxWidth() const
{
    return MaxWidth;
}

void ViewInfo::SetMaxWidth(float maxwidth)
{
    MaxWidth = maxwidth;
}

float ViewInfo::GetMinHeight() const
{
    return MinHeight;
}

void ViewInfo::SetMinHeight(float minheight)
{
    MinHeight = minheight;
}

float ViewInfo::GetMinWidth() const
{
    return MinWidth;
}

void ViewInfo::SetMinWidth(float minwidth)
{
    MinWidth = minwidth;
}

float ViewInfo::GetAlpha() const
{
    return Alpha;
}

void ViewInfo::SetAlpha(float alpha)
{
    Alpha = alpha;
}

float ViewInfo::GetX() const
{
    return X;
}

void ViewInfo::SetX(float x)
{
    X = x;
}

void ViewInfo::SetXOffset(float offset)
{
    XOffset = offset;
}


void ViewInfo::SetXOrigin(float origin)
{
    XOrigin = origin;
}

float ViewInfo::GetY() const
{
    return Y;
}

void ViewInfo::SetY(float y)
{
    Y = y;
}

void ViewInfo::SetYOffset(float offset)
{
    YOffset = offset;
}


void ViewInfo::SetYOrigin(float origin)
{
    YOrigin = origin;
}

float ViewInfo::GetRawYOrigin()
{
    return YOrigin;
}
float ViewInfo::GetRawXOrigin()
{
    return XOrigin;
}

float ViewInfo::GetRawWidth()
{
    return Width;
}

float ViewInfo::GetRawHeight()
{
    return Height;
}

void ViewInfo::SetHeight(float height)
{
    Height = height;
}

void ViewInfo::SetWidth(float width)
{
    Width = width;
}

float ViewInfo::GetFontSize() const
{
    if(FontSize == -1)
    {
        return GetHeight();
    }
    else
    {
        return FontSize;
    }
}

void ViewInfo::SetFontSize(float fontSize)
{
    FontSize = fontSize;
}


float ViewInfo::GetBackgroundRed()
{
    return BackgroundRed;
}

void ViewInfo::SetBackgroundRed(float value)
{
    BackgroundRed = value;
}

float ViewInfo::GetBackgroundGreen()
{
    return BackgroundGreen;
}

void ViewInfo::SetBackgroundGreen(float value)
{
    BackgroundGreen = value;
}
float ViewInfo::GetBackgroundBlue()
{
    return BackgroundBlue;
}

void ViewInfo::SetBackgroundBlue(float value)
{
    BackgroundBlue = value;
}

float ViewInfo::GetBackgroundAlpha()
{
    return BackgroundAlpha;
}

void ViewInfo::SetBackgroundAlpha(float value)
{
    BackgroundAlpha = value;
}