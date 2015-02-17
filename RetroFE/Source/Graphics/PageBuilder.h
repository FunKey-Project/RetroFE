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

#include "Component/Image.h"
#include "FontCache.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <rapidxml.hpp>
#include <vector>

class ScrollingList;
class Page;
class ViewInfo;
class Configuration;

class PageBuilder
{
public:
    PageBuilder(std::string layoutKey, Configuration &c, FontCache *fc);
    virtual ~PageBuilder();
    Page *BuildPage();

private:
    std::string LayoutKey;
    std::string LayoutPath;
    Configuration &Config;
    float ScaleX;
    float ScaleY;->
    int ScreenHeight;
    int ScreenWidth;
    SDL_Color FontColor;
    std::string Font;
    int FontSize;
    FontCache *FC;

    void LoadReloadableImages(rapidxml::xml_node<> *layout, std::string tagName, Page *page);
    float GetVerticalAlignment(rapidxml::xml_attribute<> *attribute, float valueIfNull);
    float GetHorizontalAlignment(rapidxml::xml_attribute<> *attribute, float valueIfNull);
    void BuildViewInfo(rapidxml::xml_node<> *componentXml, ViewInfo *info, rapidxml::xml_node<> *defaultXml = NULL);
    bool BuildComponents(rapidxml::xml_node<> *layout, Page *page);
    void LoadTweens(Component *c, rapidxml::xml_node<> *componentXml);
    TweenSets CreateTweenInstance(rapidxml::xml_node<> *componentXml);
    void BuildTweenAttributes(TweenSets &tweens, rapidxml::xml_node<> *componentXml, std::string tagName, std::string tweenName);
    ScrollingList * BuildMenu(rapidxml::xml_node<> *menuXml);
    void BuildCustomMenu(ScrollingList *menu, rapidxml::xml_node<> *menuXml, rapidxml::xml_node<> *itemDefaults);
    void BuildVerticalMenu(ScrollingList *menu, rapidxml::xml_node<> *menuXml, rapidxml::xml_node<> *itemDefaults);
    int ParseMenuPosition(std::string strIndex);
    rapidxml::xml_attribute<> *FindAttribute(rapidxml::xml_node<> *componentXml, std::string attribute, rapidxml::xml_node<> *defaultXml);
    void GetTweenAttributes(rapidxml::xml_node<> *node, std::vector<std::vector<Tween>> &TweenAttributes);
    void GetTweenSets(rapidxml::xml_node<> *node, std::vector<Tween> &tweens);
    ViewInfo CreateMenuItemInfo(rapidxml::xml_node<> *component, rapidxml::xml_node<> *defaults, float y);
};
