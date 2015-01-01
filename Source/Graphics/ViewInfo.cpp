/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
, Transparency(1)
, Layer(0)
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
   float value = Height;

   if(Height == -1 && Width == -1)
   {
      value = ImageHeight;
   }
   else
   {
      if (Height == -1 && ImageWidth != 0)
      {
         value = ImageHeight * Width / ImageWidth;
      }

      if (value < MinHeight)
      {
         value = MinHeight;
      }
      else if (value > MaxHeight)
      {
         value = MaxHeight;
      }
   }

   return value;
}

float ViewInfo::GetWidth() const
{
   float value = Width;

   if(Height == -1 && Width == -1)
   {
      value = ImageWidth;
   }
   else
   {
      if (Width == -1 && ImageHeight != 0)
      {
         value = ImageWidth * Height / ImageHeight;
      }
      if (value < MinWidth)
      {
         value = MinWidth;
      }
      else if (value > MaxWidth)
      {
         value = MaxWidth;
      }
   }

   return value;
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

float ViewInfo::GetTransparency() const
{
   return Transparency;
}

void ViewInfo::SetTransparency(float transparency)
{
   Transparency = transparency;
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
