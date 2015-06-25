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

#include "VideoBuilder.h"
#include "../../Utility/Utils.h"
#include "../../Utility/Log.h"
#include "../../Video/VideoFactory.h"
#include <fstream>


VideoComponent * VideoBuilder::createVideo(std::string path, std::string name, float scaleX, float scaleY)
{
    VideoComponent *component = NULL;
    std::vector<std::string> extensions;

    extensions.push_back("mp4");
    extensions.push_back("MP4");
    extensions.push_back("avi");
    extensions.push_back("AVI");

    std::string prefix = Utils::combinePath(path, name);
    std::string file;

    if(Utils::findMatchingFile(prefix, extensions, file))
    {
        IVideo *video = factory_.createVideo();

        if(video)
        {
            component = new VideoComponent(video, file, scaleX, scaleY);
        }
    }

    return component;
}

