/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
   float GetTransparency() const;
   void SetTransparency(float transparency);
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
   float Transparency;
   unsigned int Layer;
   float HorizontalScale;
   float VerticalScale;
};
