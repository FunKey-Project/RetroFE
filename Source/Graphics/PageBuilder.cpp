/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "PageBuilder.h"
#include "Page.h"
#include "ViewInfo.h"
#include "Component/Image.h"
#include "Component/Text.h"
#include "Component/ReloadableText.h"
#include "Component/ReloadableMedia.h"
#include "Component/ScrollingList.h"
#include "Animate/TweenTypes.h"
#include "../Sound/Sound.h"
#include "../Collection/Item.h"
#include "../SDL.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include <algorithm>
#include <cfloat>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace rapidxml;


PageBuilder::PageBuilder(std::string layoutKey, std::string collection, Configuration *c, FontCache *fc)
: LayoutKey(layoutKey)
, Collection(collection)
, Config(c)
, ScaleX(1)
, ScaleY(1)
, ScreenHeight(0)
, ScreenWidth(0)
, FC(fc)
{
   ScreenWidth = SDL::GetWindowWidth();
   ScreenHeight = SDL::GetWindowHeight();
   FontColor.a = 255;
   FontColor.r = 255;
   FontColor.g = 0;
   FontColor.b = 0;
}

PageBuilder::~PageBuilder()
{
}

Page *PageBuilder::BuildPage()
{
   Page *page = NULL;

	std::string layoutFile;
	std::string layoutName = LayoutKey;

   LayoutPath = Configuration::GetAbsolutePath() + "/Layouts/" + layoutName;
   layoutFile = LayoutPath + "/Layout.xml";

   Logger::Write(Logger::ZONE_INFO, "Layout", "Initializing " + layoutFile);

   rapidxml::xml_document<> doc;
   std::ifstream file(layoutFile.c_str());
   std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

   if(!file.good())
   {
      Logger::Write(Logger::ZONE_INFO, "Layout", "could not find layout file: " + layoutFile);
      return NULL;
   }

	try
   {
      buffer.push_back('\0');

      doc.parse<0>(&buffer[0]);

      xml_node<> *root = doc.first_node("layout");


      if(!root)
      {
         Logger::Write(Logger::ZONE_ERROR, "Layout", "Missing <layout> tag");
         return NULL;
      }
      else
      {
         xml_attribute<> *layoutWidthXml = root->first_attribute("width");
         xml_attribute<> *layoutHeightXml = root->first_attribute("height");
         xml_attribute<> *fontXml = root->first_attribute("font");
         xml_attribute<> *fontColorXml = root->first_attribute("fontColor");
         int layoutHeight;
         int layoutWidth;
         if(!layoutWidthXml || !layoutHeightXml)
         {
            Logger::Write(Logger::ZONE_ERROR, "Layout", "<layout> tag must specify a width and height");
            return NULL;
         }
         if(fontXml)
         {
            //todo: reuse from ComponentBuilder. Not sure how since it relies on knowing the collection
            std::string fontPropertyKey  = "layouts." + LayoutKey + ".font";
            Config->SetProperty(fontPropertyKey, fontXml->value());

            Font = Config->ConvertToAbsolutePath(
                     Config->GetAbsolutePath() + "/Layouts/" + LayoutKey + "/",
                     fontXml->value());

            Logger::Write(Logger::ZONE_DEBUG, "Layout", "Layout font set to " + Font);

         }

         if(fontColorXml)
         {
            int intColor = 0;
            std::stringstream ss;
            ss << std::hex << fontColorXml->value();
            ss >> intColor;

            FontColor.b = intColor & 0xFF;
            intColor >>= 8;
            FontColor.g = intColor & 0xFF;
            intColor >>= 8;
            FontColor.r = intColor & 0xFF;
         }

         layoutWidth = Utils::ConvertInt(layoutWidthXml->value());
         layoutHeight = Utils::ConvertInt(layoutHeightXml->value());

         if(layoutWidth == 0 || layoutHeight == 0)
         {
            Logger::Write(Logger::ZONE_ERROR, "Layout", "Layout width and height cannot be set to 0");
            return NULL;
         }

         ScaleX = (float)ScreenWidth / (float)layoutWidth;
         ScaleY = (float)ScreenHeight / (float)layoutHeight;

         std::stringstream ss;
         ss << layoutWidth << "x" << layoutHeight << " (scale " << ScaleX << "x" << ScaleY << ")";
         Logger::Write(Logger::ZONE_DEBUG, "Layout", "Layout resolution " + ss.str());

         page = new Page(Collection);

         // load sounds
         for(xml_node<> *sound = root->first_node("sound"); sound; sound = sound->next_sibling("sound"))
         {
            xml_attribute<> *src = sound->first_attribute("src");
            xml_attribute<> *type = sound->first_attribute("type");
            std::string file = Configuration::ConvertToAbsolutePath(LayoutPath, src->value());
            if(!type)
            {
               Logger::Write(Logger::ZONE_ERROR, "Layout", "Sound tag missing type attribute");
            }
            else
            {
               Sound *sound = new Sound(file);
               std::string soundType = type->value();

               if(!soundType.compare("load"))
               {
                  page->SetLoadSound(sound);
               }
               else if(!soundType.compare("unload"))
               {
                  page->SetUnloadSound(sound);
               }
               else if(!soundType.compare("highlight"))
               {
                  page->SetHighlightSound(sound);
               }
               else if(!soundType.compare("select"))
               {
                  page->SetSelectSound(sound);
               }
               else
               {
                  Logger::Write(Logger::ZONE_WARNING, "Layout", "Unsupported sound effect type \"" + soundType + "\"");
               }
            }
         }
         if(!BuildComponents(root, page))
         {
            delete page;
            page = NULL;
         }

      }

   }
	catch(rapidxml::parse_error &e)
	{
      std::string what = e.what();
      long line = static_cast<long>(std::count(&buffer.front(), e.where<char>(), char('\n')) + 1);
      std::stringstream ss;
      ss << "Could not parse layout file. [Line: " << line << "] Reason: " << e.what();

      Logger::Write(Logger::ZONE_ERROR, "Layout", ss.str());
	}
	catch(std::exception &e)
	{
	   std::string what = e.what();
      Logger::Write(Logger::ZONE_ERROR, "Layout", "Could not parse layout file. Reason: " + what);
	}

	if(page)
	{
	   Logger::Write(Logger::ZONE_DEBUG, "Layout", "Created page");
	}

	return page;
}



float PageBuilder::GetHorizontalAlignment(xml_attribute<> *attribute, float valueIfNull)
{
   float value;
   std::string str;

   if(!attribute)
   {
      value = valueIfNull;
   }
   else
   {
      str = attribute->value();

      if(!str.compare("left"))
      {
         value = 0;
      }
      else if(!str.compare("center"))
      {
         value = static_cast<float>(ScreenWidth) / 2;
      }
      else if(!str.compare("right") || !str.compare("stretch"))
      {
         value = static_cast<float>(ScreenWidth);
      }
      else
      {
         value = Utils::ConvertFloat(str) * ScaleX;
      }
   }

   return value;
}

float PageBuilder::GetVerticalAlignment(xml_attribute<> *attribute, float valueIfNull)
{
   float value;
   std::string str;
   if(!attribute)
   {
      value = valueIfNull;
   }
   else
   {
      str = attribute->value();

      if(!str.compare("top"))
      {
         value = 0;
      }
      else if(!str.compare("center"))
      {
         value = static_cast<float>(ScreenHeight / 2);
      }
      else if(!str.compare("bottom") || !str.compare("stretch"))
      {
         value = static_cast<float>(ScreenHeight);
      }
      else
      {
         value = Utils::ConvertFloat(str) * ScaleY;
      }
   }
   return value;
}




bool PageBuilder::BuildComponents(xml_node<> *layout, Page *page)
{
   bool retVal = true;
   xml_node<> *menuXml = layout->first_node("menu");
   if(!menuXml)
   {
      Logger::Write(Logger::ZONE_ERROR, "Layout", "Missing menu tag");
      retVal = false;
   }
   else if(menuXml)
   {
      ScrollingList *scrollingList = BuildCustomMenu(menuXml);
      page->SetMenu(scrollingList);

      for(xml_node<> *componentXml = layout->first_node("image"); componentXml; componentXml = componentXml->next_sibling("image"))
      {
         xml_attribute<> *src = componentXml->first_attribute("src");

         if (!src)
         {
            Logger::Write(Logger::ZONE_ERROR, "Layout", "Image component in layout does not specify a source image file");
         }
         else
         {
            std::string imagePath;
            imagePath = Configuration::ConvertToAbsolutePath(LayoutPath, imagePath);

            imagePath.append("/");
            imagePath.append(src->value());



            Image *c = new Image(imagePath, ScaleX, ScaleY);
            ViewInfo *v = c->GetBaseViewInfo();
            BuildViewInfo(componentXml, v);
            LoadTweens(c, componentXml);
            page->AddComponent(c);
         }
      }

      for(xml_node<> *componentXml = layout->first_node("text"); componentXml; componentXml = componentXml->next_sibling("text"))
      {
         xml_attribute<> *value = componentXml->first_attribute("value");

         if (!value)
         {
            Logger::Write(Logger::ZONE_WARNING, "Layout", "Text component in layout does not specify a value");
         }
         else
         {
        	FC->LoadFont(Font, FontColor);
            Text *c = new Text(value->value(), FC->GetFont(Font), FontColor, ScaleX, ScaleY);
            ViewInfo *v = c->GetBaseViewInfo();

            BuildViewInfo(componentXml, v);

            LoadTweens(c, componentXml);
            page->AddComponent(c);
         }
      }

      LoadReloadableImages(layout, "reloadableImage", page);
      LoadReloadableImages(layout, "reloadableVideo", page);
      LoadReloadableImages(layout, "reloadableText", page);
   }

   return retVal;
}

void PageBuilder::LoadReloadableImages(xml_node<> *layout, std::string tagName, Page *page)
{
   for(xml_node<> *componentXml = layout->first_node(tagName.c_str()); componentXml; componentXml = componentXml->next_sibling(tagName.c_str()))
   {
      std::string reloadableImagePath;
      std::string reloadableVideoPath;
      xml_attribute<> *type = componentXml->first_attribute("type");

      if(tagName == "reloadableVideo")
      {
         type = componentXml->first_attribute("imageType");
      }


      if(!type && tagName == "reloadableVideo")
      {
         Logger::Write(Logger::ZONE_WARNING, "Layout", "<reloadableImage> component in layout does not specify an imageType for when the video does not exist");
      }
      if(!type && (tagName == "reloadableImage" || tagName == "reloadableText"))
      {
         Logger::Write(Logger::ZONE_ERROR, "Layout", "Image component in layout does not specify a source image file");
      }

      if(type && (tagName == "reloadableVideo" || tagName == "reloadableImage"))
      {
         std::string configImagePath = "collections." + Collection + ".media." + type->value();
         if(!Config->GetPropertyAbsolutePath(configImagePath, reloadableImagePath))
         {
            Logger::Write(Logger::ZONE_ERROR, "Layout", "Cannot process reloadable images because property \"" + configImagePath + "\" does not exist");
         }

         std::string configVideoPath = "collections." + Collection + ".media.video";

         if(!Config->GetPropertyAbsolutePath(configVideoPath, reloadableVideoPath))
         {
            Logger::Write(Logger::ZONE_WARNING, "Layout", "Could not find videos folder as \"" + configVideoPath + "\" does not exist");
         }
      }


      Component *c = NULL;

      if(tagName == "reloadableText")
      {
         if(type)
         {
            FC->LoadFont(Font, FontColor);
            c = new ReloadableText(type->value(), FC->GetFont(Font), FontColor, LayoutKey, Collection, ScaleX, ScaleY);
         }
      }
      else
      {
         c = new ReloadableMedia(reloadableImagePath, reloadableVideoPath, (tagName == "reloadableVideo"), ScaleX, ScaleY);
      }

      if(c)
      {
         LoadTweens(c, componentXml);

         page->AddComponent(c);
      }
   }
}
void PageBuilder::LoadTweens(Component *c, xml_node<> *componentXml)
{
   ViewInfo *v = c->GetBaseViewInfo();

   BuildViewInfo(componentXml, v);

   Component::TweenSets *tweenSets;
   tweenSets = new std::vector<std::vector<Tween *> *>();
   GetTweenSets(componentXml->first_node("onEnter"), tweenSets);
   c->SetOnEnterTweens(tweenSets);

   tweenSets = new std::vector<std::vector<Tween *> *>();
   GetTweenSets(componentXml->first_node("onExit"), tweenSets);
   c->SetOnExitTweens(tweenSets);

   tweenSets = new std::vector<std::vector<Tween *> *>();
   GetTweenSets(componentXml->first_node("onIdle"), tweenSets);
   c->SetOnIdleTweens(tweenSets);

   tweenSets = new std::vector<std::vector<Tween *> *>();
   GetTweenSets(componentXml->first_node("onHighlightEnter"), tweenSets);
   c->SetOnHighlightEnterTweens(tweenSets);

   tweenSets = new std::vector<std::vector<Tween *> *>();
   GetTweenSets(componentXml->first_node("onHighlightExit"), tweenSets);
   c->SetOnHighlightExitTweens(tweenSets);
}


ScrollingList * PageBuilder::BuildCustomMenu(xml_node<> *menuXml)
{
   ScrollingList *menu = NULL;
   std::string imageType="null";

   xml_attribute<> *imageTypeXml = menuXml->first_attribute("imageType");

   if(imageTypeXml)
   {
      imageType = imageTypeXml->value();
   }

   FC->LoadFont(Font, FontColor);
   menu = new ScrollingList(Config, ScaleX, ScaleY, FC->GetFont(Font), FontColor, LayoutKey, Collection, imageType);

   ViewInfo *v = menu->GetBaseViewInfo();
   BuildViewInfo(menuXml, v);

   std::vector<ViewInfo *> *points = new std::vector<ViewInfo *>();

   int i = 0;
   for(xml_node<> *componentXml = menuXml->first_node("item"); componentXml; componentXml = componentXml->next_sibling("item"))
   {
      ViewInfo *viewInfo = new ViewInfo();
      BuildViewInfo(componentXml, viewInfo);

      points->push_back(viewInfo);

      xml_attribute<> *selected = componentXml->first_attribute("selected");

      if(selected)
      {
         menu->SetSelectedIndex(i);
      }

      i++;
   }

   menu->SetPoints(points);


   return menu;
}


xml_attribute<> *PageBuilder::FindRecursiveAttribute(xml_node<> *componentXml, std::string attribute)
{

   xml_attribute<> *attributeXml = NULL;
   xml_node<> *parent = componentXml->parent();

   // root xml node height and width attributes are to define the layout size itself, not the elements
   if(parent && parent->parent())
   {
      attributeXml = componentXml->first_attribute(attribute.c_str());

      if(!attributeXml)
      {
          attributeXml = FindRecursiveAttribute(parent, attribute);
      }
   }

   return attributeXml;
}

void PageBuilder::BuildViewInfo(xml_node<> *componentXml, ViewInfo *info)
{
   xml_attribute<> *x = FindRecursiveAttribute(componentXml, "x");
   xml_attribute<> *y = FindRecursiveAttribute(componentXml, "y");
   xml_attribute<> *xOffset = FindRecursiveAttribute(componentXml, "xOffset");
   xml_attribute<> *yOffset = FindRecursiveAttribute(componentXml, "yOffset");
   xml_attribute<> *xOrigin = FindRecursiveAttribute(componentXml, "xOrigin");
   xml_attribute<> *yOrigin = FindRecursiveAttribute(componentXml, "yOrigin");
   xml_attribute<> *height = FindRecursiveAttribute(componentXml, "height");
   xml_attribute<> *width = FindRecursiveAttribute(componentXml, "width");
   xml_attribute<> *fontSize = FindRecursiveAttribute(componentXml, "fontSize");
   xml_attribute<> *minHeight = FindRecursiveAttribute(componentXml, "minHeight");
   xml_attribute<> *minWidth = FindRecursiveAttribute(componentXml, "minWidth");
   xml_attribute<> *maxHeight = FindRecursiveAttribute(componentXml, "maxHeight");
   xml_attribute<> *maxWidth = FindRecursiveAttribute(componentXml, "maxWidth");
   xml_attribute<> *transparency = FindRecursiveAttribute(componentXml, "transparency");
   xml_attribute<> *angle = FindRecursiveAttribute(componentXml, "angle");
   xml_attribute<> *layer = FindRecursiveAttribute(componentXml, "layer");

   info->SetX(GetHorizontalAlignment(x, 0));
   info->SetY(GetVerticalAlignment(y, 0));


   info->SetXOffset( GetHorizontalAlignment(xOffset, 0));
   info->SetYOffset( GetVerticalAlignment(yOffset, 0));
   float xOriginRelative = GetHorizontalAlignment(xOrigin, 0);
   float yOriginRelative = GetVerticalAlignment(yOrigin, 0);

   // the origins need to be saved as a percent since the heights and widths can be scaled
   info->SetXOrigin(xOriginRelative / ScreenWidth);
   info->SetYOrigin(yOriginRelative / ScreenHeight);


   if(!height && !width)
   {
      info->SetHeight(-1);
      info->SetWidth(-1);
   }
   else
   {
      info->SetHeight(GetVerticalAlignment(height, -1));
      info->SetWidth(GetHorizontalAlignment(width, -1));
   }
   info->SetFontSize(GetVerticalAlignment(fontSize, -1));
   /*
   std::stringstream ss;
   ss << "font size is \"" << info->GetFontSize() << "\"";
   Logger::Write(Logger::ZONE_ERROR, "Layout", ss.str());
   */
   info->SetMinHeight(GetVerticalAlignment(minHeight, 0));
   info->SetMinWidth(GetHorizontalAlignment(minWidth, 0));
   info->SetMaxHeight(GetVerticalAlignment(maxHeight, FLT_MAX));
   info->SetMaxWidth(GetVerticalAlignment(maxWidth, FLT_MAX));
   info->SetTransparency( transparency ? Utils::ConvertFloat(transparency->value()) : 1);
   info->SetAngle( angle ? Utils::ConvertFloat(angle->value()) : 0);
   info->SetLayer( layer ? Utils::ConvertInt(layer->value()) : 0);
}


void PageBuilder::GetTweenSets(xml_node<> *node, std::vector<std::vector<Tween *> *> *tweenSets)
{
   if(node)
   {
      for(xml_node<> *set = node->first_node("set"); set; set = set->next_sibling("set"))
      {
         std::vector<Tween *> *tweens = new std::vector<Tween *>();
         GetTweenSet(set, *tweens);
         tweenSets->push_back(tweens);
      }
   }
}

void PageBuilder::GetTweenSet(xml_node<> *node, std::vector<Tween *> &tweens)
{
   xml_attribute<> *durationXml = node->first_attribute("duration");

   if(!durationXml)
   {
      Logger::Write(Logger::ZONE_ERROR, "Layout", "Animation set tag missing \"duration\" attribute");
   }
   else
   {
      for(xml_node<> *animate = node->first_node("animate"); animate; animate = animate->next_sibling("animate"))
      {
         xml_attribute<> *type = animate->first_attribute("type");
         xml_attribute<> *from = animate->first_attribute("from");
         xml_attribute<> *to = animate->first_attribute("to");
         xml_attribute<> *algorithmXml = animate->first_attribute("algorithm");

         if(!type)
         {
            Logger::Write(Logger::ZONE_ERROR, "Layout", "Animate tag missing \"type\" attribute");
         }
         else if(!from)
         {
            Logger::Write(Logger::ZONE_ERROR, "Layout", "Animate tag missing \"from\" attribute");
         }
         else if(!to)
         {
            Logger::Write(Logger::ZONE_ERROR, "Layout", "Animate tag missing \"to\" attribute");
         }
         else
         {
            float fromValue = Utils::ConvertFloat(from->value());
            float toValue = Utils::ConvertFloat(to->value());
            float durationValue = Utils::ConvertFloat(durationXml->value());

            TweenAlgorithm algorithm = LINEAR;
            TweenProperty property;

            if(algorithmXml)
            {
               algorithm = Tween::GetTweenType(algorithmXml->value());

            }

            if(Tween::GetTweenProperty(type->value(), property))
            {
               switch(property)
               {
                  case TWEEN_PROPERTY_WIDTH:
                  case TWEEN_PROPERTY_X:
                  case TWEEN_PROPERTY_X_OFFSET:
                     fromValue = GetHorizontalAlignment(from, 0);
                     toValue = GetHorizontalAlignment(to, 0);
                     break;

                     // x origin gets translated to a percent
                  case TWEEN_PROPERTY_X_ORIGIN:
                     fromValue = GetHorizontalAlignment(from, 0) / ScreenWidth;
                     toValue = GetHorizontalAlignment(to, 0) / ScreenWidth;
                     break;

                  case TWEEN_PROPERTY_HEIGHT:
                  case TWEEN_PROPERTY_Y:
                  case TWEEN_PROPERTY_Y_OFFSET:
                  case TWEEN_PROPERTY_FONT_SIZE:
                     fromValue = GetVerticalAlignment(from, 0);
                     toValue = GetVerticalAlignment(to, 0);
                     break;

                  // y origin gets translated to a percent
                  case TWEEN_PROPERTY_Y_ORIGIN:
                     fromValue = GetVerticalAlignment(from, 0) / ScreenHeight;
                     toValue = GetVerticalAlignment(to, 0) / ScreenHeight;
                     break;

                  default:
                     break;
               }

               Tween *t = new Tween(property, algorithm, fromValue, toValue, durationValue);
               tweens.push_back(t);
            }
            else
            {
               std::stringstream ss;
               ss << "Unsupported tween type attribute \"" << type->value() << "\"";
               Logger::Write(Logger::ZONE_ERROR, "Layout", ss.str());
            }
         }
      }
   }
}
