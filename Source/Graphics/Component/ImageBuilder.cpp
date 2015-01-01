/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "ImageBuilder.h"
#include "../../Utility/Utils.h"
#include "../../Utility/Log.h"
#include <fstream>

Image * ImageBuilder::CreateImage(std::string path, std::string name, float scaleX, float scaleY)
{
   Image *image = NULL;
   std::vector<std::string> extensions;

   extensions.push_back("png");
   extensions.push_back("PNG");
   extensions.push_back("jpg");
   extensions.push_back("JPG");
   extensions.push_back("jpeg");
   extensions.push_back("JPEG");

   std::string prefix = path + "/" + name;
   std::string file;

   if(Utils::FindMatchingFile(prefix, extensions, file))
   {
      image = new Image(file, scaleX, scaleY);
   }

   return image;
}
