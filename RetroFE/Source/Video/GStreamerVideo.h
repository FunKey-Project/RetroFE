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

#include "IVideo.h"

extern "C"
{
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
}


class GStreamerVideo : public IVideo
{
public:
    GStreamerVideo();
    ~GStreamerVideo();
    bool Initialize();
    bool Play(std::string file);
    bool Stop();
    bool DeInitialize();
    SDL_Texture *GetTexture() const;
    void Update(float dt);
    void Draw();
    void SetNumLoops(int n);
    void FreeElements();
    int GetHeight();
    int GetWidth();

private:
    static void ProcessNewBuffer (GstElement *fakesink, GstBuffer *buf, GstPad *pad, gpointer data);
    static gboolean BusCallback(GstBus *bus, GstMessage *msg, gpointer data);

    GstElement *Playbin;
    GstElement *VideoBin;
    GstElement *VideoSink;
    GstElement *VideoConvert;
    GstCaps *VideoConvertCaps;
    GstBus *VideoBus;
    SDL_Texture* Texture;
    gint Height;
    gint Width;
    char *VideoBuffer;
    gsize VideoBufferSize;
    gsize MaxVideoBufferSize;
    bool FrameReady;
    bool IsPlaying;
    static bool Initialized;
    int PlayCount;
    std::string CurrentFile;
    int NumLoops;
};
