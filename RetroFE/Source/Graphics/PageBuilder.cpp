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

#include "PageBuilder.h"
#include "Page.h"
#include "ViewInfo.h"
#include "Component/Container.h"
#include "Component/Image.h"
#include "Component/Text.h"
#include "Component/ReloadableText.h"
#include "Component/ReloadableMedia.h"
#include "Component/ScrollingList.h"
#include "Animate/TweenSet.h"
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
#include <map>

using namespace rapidxml;

static const int MENU_FIRST = 0;   // first visible item in the list
static const int MENU_LAST = -3;   // last visible item in the list
static const int MENU_START = -1;  // first item transitions here after it scrolls "off the menu/screen"
static const int MENU_END = -2;    // last item transitions here after it scrolls "off the menu/screen"
static const int MENU_CENTER = -4;
    
//todo: this file is starting to become a god class of building. Consider splitting into sub-builders
PageBuilder::PageBuilder(std::string layoutKey, Configuration &c, FontCache *fc)
    : LayoutKey(layoutKey)
    , Config(c)
    , ScaleX(1)
    , ScaleY(1)
    , ScreenHeight(0)
    , ScreenWidth(0)
    , FontSize(24)
    , FC(fc)
{
    ScreenWidth = SDL::GetWindowWidth();
    ScreenHeight = SDL::GetWindowHeight();
    FontColor.a = 255;
    FontColor.r = 0;
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
            xml_attribute<> *fontSizeXml = root->first_attribute("loadFontSize");

            int layoutHeight;
            int layoutWidth;
            if(!layoutWidthXml || !layoutHeightXml)
            {
                Logger::Write(Logger::ZONE_ERROR, "Layout", "<layout> tag must specify a width and height");
                return NULL;
            }
            if(fontXml)
            {
                std::string fontPropertyKey  = "layouts." + LayoutKey + ".font";
                Config.SetProperty(fontPropertyKey, fontXml->value());

                Font = Config.ConvertToAbsolutePath(
                           Config.GetAbsolutePath() + "/Layouts/" + LayoutKey + "/",
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

            if(fontSizeXml)
            {
                FontSize = Utils::ConvertInt(fontSizeXml->value());
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

            page = new Page(Config);

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

    Logger::Write(Logger::ZONE_INFO, "Layout", "Initialized");

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
    for(xml_node<> *componentXml = layout->first_node("menu"); componentXml; componentXml = componentXml->next_sibling("menu"))
    {
        ScrollingList *scrollingList = BuildMenu(componentXml);
        page->PushMenu(scrollingList);
    }

    for(xml_node<> *componentXml = layout->first_node("container"); componentXml; componentXml = componentXml->next_sibling("container"))
    {
        Container *c = new Container();
        ViewInfo *v = c->GetBaseViewInfo();
        BuildViewInfo(componentXml, v);
        LoadTweens(c, componentXml);
        page->AddComponent(c);
    }


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
            FC->LoadFont(Font, FontSize, FontColor);
            Text *c = new Text(value->value(), FC->GetFont(Font), FontColor, ScaleX, ScaleY);
            ViewInfo *v = c->GetBaseViewInfo();

            BuildViewInfo(componentXml, v);

            LoadTweens(c, componentXml);
            page->AddComponent(c);
        }
    }
    
    for(xml_node<> *componentXml = layout->first_node("statusText"); componentXml; componentXml = componentXml->next_sibling("statusText"))
    {
        FC->LoadFont(Font, FontSize, FontColor);
        Text *c = new Text("", FC->GetFont(Font), FontColor, ScaleX, ScaleY);
        ViewInfo *v = c->GetBaseViewInfo();

        BuildViewInfo(componentXml, v);

        LoadTweens(c, componentXml);
        page->AddComponent(c);
        page->SetStatusTextComponent(c);
    }


    LoadReloadableImages(layout, "reloadableImage", page);
    LoadReloadableImages(layout, "reloadableVideo", page);
    LoadReloadableImages(layout, "reloadableText", page);

    return true;
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



        Component *c = NULL;

        if(tagName == "reloadableText")
        {
            if(type)
            {
                FC->LoadFont(Font, FontSize, FontColor);
                c = new ReloadableText(type->value(), FC->GetFont(Font), FontColor, LayoutKey, ScaleX, ScaleY);
            }
        }
        else
        {
            c = new ReloadableMedia(Config, type->value(), (tagName == "reloadableVideo"), ScaleX, ScaleY);
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

    c->SetTweens(CreateTweenInstance(componentXml));
}

TweenSet *PageBuilder::CreateTweenInstance(xml_node<> *componentXml)
{
    TweenSet *tweens = new TweenSet();

    GetTweenSets(componentXml->first_node("onEnter"), tweens->GetOnEnterTweens());
    GetTweenSets(componentXml->first_node("onExit"), tweens->GetOnExitTweens());
    GetTweenSets(componentXml->first_node("onIdle"), tweens->GetOnIdleTweens());
    GetTweenSets(componentXml->first_node("onHighlightEnter"), tweens->GetOnHighlightEnterTweens());
    GetTweenSets(componentXml->first_node("onHighlightExit"), tweens->GetOnHighlightExitTweens());

    for(xml_node<> *menuEnter = componentXml->first_node("onMenuEnter"); menuEnter; menuEnter = menuEnter->next_sibling("onMenuEnter"))
    {
        xml_attribute<> *indexXml = menuEnter->first_attribute("menuIndex");
        int index = (indexXml) ? Utils::ConvertInt(indexXml->value()) : -1;

        TweenSet::TweenSets *sets = new TweenSet::TweenSets();
        GetTweenSets(menuEnter, sets);
        tweens->SetOnMenuEnterTweens(index, sets);
    }   

    for(xml_node<> *menuExit = componentXml->first_node("onMenuExit"); menuExit; menuExit = menuExit->next_sibling("onMenuExit"))
    {
        xml_attribute<> *indexXml = menuExit->first_attribute("menuIndex");
        int index = (indexXml) ? Utils::ConvertInt(indexXml->value()) : -1;

        TweenSet::TweenSets *sets = new TweenSet::TweenSets();
        GetTweenSets(menuExit, sets);
        tweens->SetOnMenuExitTweens(index, sets);
    }


    return tweens;
}


ScrollingList * PageBuilder::BuildMenu(xml_node<> *menuXml)
{
    ScrollingList *menu = NULL;
    std::string menuType = "vertical";
    std::string imageType = "null";
    std::map<int, xml_node<> *> overrideItems;
    xml_node<> *itemDefaults = menuXml->first_node("itemDefaults");
    xml_attribute<> *imageTypeXml = menuXml->first_attribute("imageType");
    xml_attribute<> *menuTypeXml = menuXml->first_attribute("type");

    if(menuTypeXml)
    {
        menuType = menuTypeXml->value();
    }

    // ensure <menu> has an <itemDefaults> tag
    if(!itemDefaults)
    {
        Logger::Write(Logger::ZONE_WARNING, "Layout", "Menu tag is missing <itemDefaults> tag.");
    }

    if(imageTypeXml)
    {
        imageType = imageTypeXml->value();
    }

    // on default, text will be rendered to the menu. Preload it into cache.
    FC->LoadFont(Font, FontSize, FontColor);

    menu = new ScrollingList(Config, ScaleX, ScaleY, FC->GetFont(Font), FontColor, LayoutKey, imageType);

    ViewInfo *v = menu->GetBaseViewInfo();
    BuildViewInfo(menuXml, v);

    if(menuType == "custom")
    {
        BuildCustomMenu(menu, menuXml, itemDefaults);
    }
    else
    {
        BuildVerticalMenu(menu, menuXml, itemDefaults);
    }

    LoadTweens(menu, menuXml);

    return menu;
}


void PageBuilder::BuildCustomMenu(ScrollingList *menu, xml_node<> *menuXml, xml_node<> *itemDefaults)
{
    std::vector<ViewInfo *> *points = new std::vector<ViewInfo *>();
    std::vector<TweenSet *> *tweenPoints = new std::vector<TweenSet *>();

    int i = 0;

    for(xml_node<> *componentXml = menuXml->first_node("item"); componentXml; componentXml = componentXml->next_sibling("item"))
    {
        ViewInfo *viewInfo = new ViewInfo();
        BuildViewInfo(componentXml, viewInfo, itemDefaults);
        
        points->push_back(viewInfo);
        tweenPoints->push_back(CreateTweenInstance(componentXml));
        xml_attribute<> *selected = componentXml->first_attribute("selected");

        if(selected)
        {
            menu->SetSelectedIndex(i);
        }

        i++;
    }

    menu->SetPoints(points, tweenPoints);
}

void PageBuilder::BuildVerticalMenu(ScrollingList *menu, xml_node<> *menuXml, xml_node<> *itemDefaults)
{
    std::vector<ViewInfo *> *points = new std::vector<ViewInfo *>();
    std::vector<TweenSet *> *tweenPoints = new std::vector<TweenSet *>();

    int selectedIndex = MENU_FIRST;
    std::map<int, xml_node<> *> overrideItems;

    // By default the menu will automatically determine the offsets for your list items.
    // We can override individual menu points to have unique characteristics (i.e. make the first item opaque or
    // make the selected item a different color).
    for(xml_node<> *componentXml = menuXml->first_node("item"); componentXml; componentXml = componentXml->next_sibling("item"))
    {
        xml_attribute<> *xmlIndex = componentXml->first_attribute("index");

        if(xmlIndex)
        {
            int itemIndex = ParseMenuPosition(xmlIndex->value());
            overrideItems[itemIndex] = componentXml;

            // check to see if the item specified is the selected index
            xml_attribute<> *xmlSelectedIndex = componentXml->first_attribute("selected");

            if(xmlSelectedIndex)
            {
                selectedIndex = itemIndex;
            }
        }
    }

    bool end = false;

    //menu start

    float height = 0;
    int index = 0;

    if(overrideItems.find(MENU_START) != overrideItems.end())
    {
        xml_node<> *component = overrideItems[MENU_START];
        ViewInfo *viewInfo = CreateMenuItemInfo(component, itemDefaults, menu->GetBaseViewInfo()->GetY() + height);
        points->push_back(viewInfo);
        tweenPoints->push_back(CreateTweenInstance(component));
    }
    while(!end)
    {
        ViewInfo *viewInfo = new ViewInfo();
        xml_node<> *component = itemDefaults;

        // uss overridden item setting if specified by layout for the given index
        if(overrideItems.find(index) != overrideItems.end())
        {
            component = overrideItems[index];
        }

        // calculate the total height of our menu items if we can load any additional items
        BuildViewInfo(component, viewInfo, itemDefaults);
        xml_attribute<> *itemSpacingXml = component->first_attribute("spacing");
        int itemSpacing = itemSpacingXml ? Utils::ConvertInt(itemSpacingXml->value()) : 0;
        float nextHeight = height + viewInfo->GetHeight() + itemSpacing;

        if(nextHeight >= menu->GetBaseViewInfo()->GetHeight())
        {
            end = true;
        }

        // we have reached the last menuitem
        if(end && overrideItems.find(MENU_LAST) != overrideItems.end())
        {
            component = overrideItems[MENU_LAST];

            BuildViewInfo(component, viewInfo, itemDefaults);
            xml_attribute<> *itemSpacingXml = component->first_attribute("spacing");
            int itemSpacing = itemSpacingXml ? Utils::ConvertInt(itemSpacingXml->value()) : 0;
            nextHeight = height + viewInfo->GetHeight() + itemSpacing;
        }

        height = nextHeight;
        viewInfo->SetY(menu->GetBaseViewInfo()->GetY() + (float)height);
        points->push_back(viewInfo);
        tweenPoints->push_back(CreateTweenInstance(component));
        index++;
    }

    //menu end
    if(overrideItems.find(MENU_END) != overrideItems.end())
    {
        xml_node<> *component = overrideItems[MENU_END];
        ViewInfo *viewInfo = CreateMenuItemInfo(component, itemDefaults, menu->GetBaseViewInfo()->GetY() + height);
        points->push_back(viewInfo);
        tweenPoints->push_back(CreateTweenInstance(component));
    }

    if(selectedIndex >= ((int)points->size()-2))
    {
        //todo: print debug statements when out of range
        selectedIndex = 1;
    }
    else
    {
        menu->SetSelectedIndex(selectedIndex+1);
    }

    menu->SetPoints(points, tweenPoints);
}

ViewInfo *PageBuilder::CreateMenuItemInfo(xml_node<> *component, xml_node<> *defaults, float y)
{
    ViewInfo *viewInfo = new ViewInfo();
    BuildViewInfo(component, viewInfo, defaults);
    viewInfo->SetY(y);
    return viewInfo;
}

int PageBuilder::ParseMenuPosition(std::string strIndex)
{
    int index = MENU_FIRST;

    if(strIndex == "end")
    {
        index = MENU_END;
    }
    else if(strIndex == "last")
    {
        index = MENU_LAST;
    }
    else if(strIndex == "start")
    {
        index = MENU_START;
    }
    else if(strIndex == "first")
    {
        index = MENU_FIRST;
    }
    else
    {
        index = Utils::ConvertInt(strIndex);
    }
    return index;
}

xml_attribute<> *PageBuilder::FindAttribute(xml_node<> *componentXml, std::string attribute, xml_node<> *defaultXml = NULL)
{
    xml_attribute<> *attributeXml = componentXml->first_attribute(attribute.c_str());

    if(!attributeXml && defaultXml)
    {
        attributeXml = defaultXml->first_attribute(attribute.c_str());
    }

    return attributeXml;
}

void PageBuilder::BuildViewInfo(xml_node<> *componentXml, ViewInfo *info, xml_node<> *defaultXml)
{
    xml_attribute<> *x = FindAttribute(componentXml, "x", defaultXml);
    xml_attribute<> *y = FindAttribute(componentXml, "y", defaultXml);
    xml_attribute<> *xOffset = FindAttribute(componentXml, "xOffset", defaultXml);
    xml_attribute<> *yOffset = FindAttribute(componentXml, "yOffset", defaultXml);
    xml_attribute<> *xOrigin = FindAttribute(componentXml, "xOrigin", defaultXml);
    xml_attribute<> *yOrigin = FindAttribute(componentXml, "yOrigin", defaultXml);
    xml_attribute<> *height = FindAttribute(componentXml, "height", defaultXml);
    xml_attribute<> *width = FindAttribute(componentXml, "width", defaultXml);
    xml_attribute<> *fontSize = FindAttribute(componentXml, "fontSize", defaultXml);
    xml_attribute<> *minHeight = FindAttribute(componentXml, "minHeight", defaultXml);
    xml_attribute<> *minWidth = FindAttribute(componentXml, "minWidth", defaultXml);
    xml_attribute<> *maxHeight = FindAttribute(componentXml, "maxHeight", defaultXml);
    xml_attribute<> *maxWidth = FindAttribute(componentXml, "maxWidth", defaultXml);
    xml_attribute<> *alpha = FindAttribute(componentXml, "alpha", defaultXml);
    xml_attribute<> *angle = FindAttribute(componentXml, "angle", defaultXml);
    xml_attribute<> *layer = FindAttribute(componentXml, "layer", defaultXml);
    xml_attribute<> *backgroundColor = FindAttribute(componentXml, "backgroundColor", defaultXml);
    xml_attribute<> *backgroundAlpha = FindAttribute(componentXml, "backgroundAlpha", defaultXml);

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
    info->SetMinHeight(GetVerticalAlignment(minHeight, 0));
    info->SetMinWidth(GetHorizontalAlignment(minWidth, 0));
    info->SetMaxHeight(GetVerticalAlignment(maxHeight, FLT_MAX));
    info->SetMaxWidth(GetVerticalAlignment(maxWidth, FLT_MAX));
    info->SetAlpha( alpha ? Utils::ConvertFloat(alpha->value()) : 1);
    info->SetAngle( angle ? Utils::ConvertFloat(angle->value()) : 0);
    info->SetLayer( layer ? Utils::ConvertInt(layer->value()) : 0);

    if(backgroundColor)
    {
        std::stringstream ss(backgroundColor->value());
        int num;
        ss >> std::hex >> num;
        int red = num / 0x10000;
        int green = (num / 0x100) % 0x100;
        int blue = num % 0x100;

        info->SetBackgroundRed(static_cast<float>(red)/255);
        info->SetBackgroundGreen(static_cast<float>(green)/255);
        info->SetBackgroundBlue(static_cast<float>(blue)/255);
    }

    if(backgroundAlpha)
    {
        info->SetBackgroundAlpha( backgroundAlpha ? Utils::ConvertFloat(backgroundAlpha->value()) : 1);
    }
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
