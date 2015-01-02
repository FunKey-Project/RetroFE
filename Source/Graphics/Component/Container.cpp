/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Container.h"
#include "../ViewInfo.h"
#include "../../SDL.h"

Container::Container()
{
    AllocateGraphicsMemory();
}

Container::~Container()
{
    FreeGraphicsMemory();
}

void Container::FreeGraphicsMemory()
{
    Component::FreeGraphicsMemory();
}

void Container::AllocateGraphicsMemory()
{
    Component::AllocateGraphicsMemory();
}

void Container::Draw()
{
    Component::Draw();
}
