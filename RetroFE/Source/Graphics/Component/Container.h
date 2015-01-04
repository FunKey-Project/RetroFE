/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Component.h"
#include <SDL2/SDL.h>
#include <string>

class Container : public Component
{
public:
    Container();
    virtual ~Container();
    void FreeGraphicsMemory();
    void AllocateGraphicsMemory();
    void Draw();
};
