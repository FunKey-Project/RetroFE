/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "VideoFactory.h"
#include "IVideo.h"
#include "GStreamerVideo.h"

bool VideoFactory::Enabled = true;
int VideoFactory::NumLoops = 0;

IVideo *VideoFactory::CreateVideo()
{
   IVideo *instance = NULL;

   if(Enabled)
   {
      instance = new GStreamerVideo();
      instance->Initialize();
      ((GStreamerVideo *)(instance))->SetNumLoops(NumLoops);
   }

   return instance;
}

void VideoFactory::SetEnabled(bool enabled)
{
   Enabled = enabled;
}

void VideoFactory::SetNumLoops(int numLoops)
{
   NumLoops = numLoops;
}
