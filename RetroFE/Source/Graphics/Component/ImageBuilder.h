/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Image.h"
#include "VideoComponent.h"
#include "../../Video/VideoFactory.h"

//todo: this is more of a factory than a builder
class ImageBuilder
{
public:
    Image * CreateImage(std::string path, std::string name, float scaleX, float scaleY);
};
