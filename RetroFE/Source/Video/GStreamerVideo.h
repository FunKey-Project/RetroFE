/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
