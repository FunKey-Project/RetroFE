/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

class IVideo;

class VideoFactory
{
public:
    IVideo *CreateVideo();
    static void SetEnabled(bool enabled);
    static void SetNumLoops(int numLoops);

private:
    static bool Enabled;
    static int NumLoops;
};
