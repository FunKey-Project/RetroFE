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
#include "Component.h"
#include "Text.h"
#include "../Font.h"
#include "../../Collection/Item.h"
#include <SDL2/SDL.h>
#include <string>

class ReloadableText : public Component
{
public:
    ReloadableText(std::string type, Font *font, std::string layoutKey, float scaleX, float scaleY);
    virtual ~ReloadableText();
    void update(float dt);
    void draw();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void launchEnter();
    void launchExit();

private:
    enum TextType
    {
        TextTypeUnknown = 0,
        TextTypeNumberButtons,
        TextTypeNumberPlayers,
        TextTypeYear,
        TextTypeTitle,
        TextTypeManufacturer,
        TextTypeGenre,
        TextTypePlaylist,
    };

    void ReloadTexture();

    Text *imageInst_;
    TextType type_;
    std::string layoutKey_;
    bool reloadRequested_;
    bool firstLoad_;
    Font *fontInst_;

    float scaleX_;
    float scaleY_;
};
