/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
    PageBuilder(std::string layoutKey, std::string collection, Configuration &c, FontCache *fc);
    virtual ~PageBuilder();
    Page *BuildPage();

private:
    std::string LayoutKey;
    std::string LayoutPath;
    std::string Collection;
    Configuration &Config;
    float ScaleX;
    float ScaleY;
    int ScreenHeight;
    int ScreenWidth;
    SDL_Color FontColor;
    std::string Font;
    int FontSize;
    FontCache *FC; //todo: don't need Font itself, just need cache instances
    void LoadReloadableImages(rapidxml::xml_node<> *layout, std::string tagName, Page *page);

    float GetVerticalAlignment(rapidxml::xml_attribute<> *attribute, float valueIfNull);
    float GetHorizontalAlignment(rapidxml::xml_attribute<> *attribute, float valueIfNull);
    void BuildViewInfo(rapidxml::xml_node<> *componentXml, ViewInfo *info, rapidxml::xml_node<> *defaultXml = NULL);
    bool BuildComponents(rapidxml::xml_node<> *layout, Page *page);
    void LoadTweens(Component *c, rapidxml::xml_node<> *componentXml);
    ScrollingList * BuildMenu(rapidxml::xml_node<> *menuXml);
    void BuildCustomMenu(ScrollingList *menu, rapidxml::xml_node<> *menuXml, rapidxml::xml_node<> *itemDefaults);
    void BuildVerticalMenu(ScrollingList *menu, rapidxml::xml_node<> *menuXml, rapidxml::xml_node<> *itemDefaults);

    int ParseMenuPosition(std::string strIndex);

    rapidxml::xml_attribute<> *FindAttribute(rapidxml::xml_node<> *componentXml, std::string attribute, rapidxml::xml_node<> *defaultXml);

    void GetTweenSets(rapidxml::xml_node<> *node, std::vector<std::vector<Tween *> *> *tweenSets);
    void GetTweenSet(rapidxml::xml_node<> *node, std::vector<Tween *> &tweens);


    void LoadLayoutXml();
    void LoadAnimations(std::string keyPrefix, Component &component, ViewInfo *defaults);
    std::vector<ViewInfo *> * BuildTweenPoints(std::string iteratorPrefix, ViewInfo *defaults);
    Component * LoadComponent(std::string keyPrefix);
    ScrollingList * LoadMenu();
    ViewInfo * CreateMenuItemInfo(rapidxml::xml_node<> *component, rapidxml::xml_node<> *defaults, float y);

    void LoadListItems(std::string keyPrefix, std::vector<ViewInfo *> *tweenPointList, ViewInfo *defaults, int &selectedItemIndex);
    void UpdateViewInfoFromTag(std::string keyPrefix, ViewInfo *p, ViewInfo *defaults);
};
