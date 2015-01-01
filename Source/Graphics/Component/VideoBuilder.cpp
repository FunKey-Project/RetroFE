/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "VideoBuilder.h"
#include "../../Utility/Utils.h"
#include "../../Utility/Log.h"
#include "../../Video/VideoFactory.h"
#include <fstream>


VideoComponent * VideoBuilder::CreateVideo(std::string path, std::string name, float scaleX, float scaleY)
{
   VideoComponent *component = NULL;
   std::vector<std::string> extensions;

   extensions.push_back("mp4");
   extensions.push_back("MP4");
   extensions.push_back("avi");
   extensions.push_back("AVI");

   std::string prefix = path + "/" + name;
   std::string file;

   if(Utils::FindMatchingFile(prefix, extensions, file))
   {
      IVideo *video = Factory.CreateVideo();

      if(video)
      {
         component = new VideoComponent(video, file, scaleX, scaleY);
      }
   }

   return component;
}

