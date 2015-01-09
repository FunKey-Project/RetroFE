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
#include "GStreamerVideo.h"
#include "../Graphics/ViewInfo.h"
#include "../Graphics/Component/Image.h"
#include "../Database/Configuration.h"
#include "../Utility/Log.h"
#include "../SDL.h"
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <gst/app/gstappsink.h>

bool GStreamerVideo::Initialized = false;

//todo: this started out as sandbox code. This class needs to be refactored

// MUST match video size
gboolean GStreamerVideo::BusCallback(GstBus *bus, GstMessage *msg, gpointer data)
{
    // this callback only needs to be defined so we can loop the video once it completes
    return TRUE;
}
GStreamerVideo::GStreamerVideo()
    : Playbin(NULL)
    , VideoBin(NULL)
    , VideoSink(NULL)
    , VideoConvert(NULL)
    , VideoConvertCaps(NULL)
    , VideoBus(NULL)
    , Texture(NULL)
    , Height(0)
    , Width(0)
    , VideoBuffer(NULL)
    , VideoBufferSize(0)
    , MaxVideoBufferSize(0)
    , FrameReady(false)
    , IsPlaying(false)
    , PlayCount(0)
    , NumLoops(0)
{
}
GStreamerVideo::~GStreamerVideo()
{
    Stop();

    if(VideoBuffer)
    {
        delete[] VideoBuffer;
        VideoBuffer = NULL;
        VideoBufferSize = 0;
        MaxVideoBufferSize = 0;
    }

    SDL_DestroyTexture(Texture);
    Texture = NULL;

    FreeElements();
}

void GStreamerVideo::SetNumLoops(int n)
{
    NumLoops = n;
}

SDL_Texture *GStreamerVideo::GetTexture() const
{
    return Texture;
}

void GStreamerVideo::ProcessNewBuffer (GstElement *fakesink, GstBuffer *buf, GstPad *new_pad, gpointer userdata)
{
    GStreamerVideo *video = (GStreamerVideo *)userdata;
    GstMapInfo map;
    SDL_LockMutex(SDL::GetMutex());

    if (!video->FrameReady && video && video->IsPlaying && gst_buffer_map (buf, &map, GST_MAP_READ))
    {
        if(!video->Width || !video->Height)
        {
            GstCaps *caps = gst_pad_get_current_caps (new_pad);
            GstStructure *s = gst_caps_get_structure(caps, 0);

            gst_structure_get_int(s, "width", &video->Width);
            gst_structure_get_int(s, "height", &video->Height);
        }

        if(video->Height && video->Width)
        {
            if(video->Texture && video->VideoBufferSize != map.size)
            {
                SDL_DestroyTexture(video->Texture);
                video->Texture = NULL;
            }

            // keep the largest video buffer allocated to avoid the penalty of reallocating and deallocating
            if(!video->VideoBuffer || video->MaxVideoBufferSize < map.size) 
            {
                if(video->VideoBuffer)
                {
                    delete[] video->VideoBuffer;
                }

                video->VideoBuffer = new char[map.size];
                video->MaxVideoBufferSize = map.size;
            }

            video->VideoBufferSize = map.size;

            memcpy(video->VideoBuffer, map.data, map.size);
            gst_buffer_unmap(buf, &map);
            video->FrameReady = true;
        }
    }
    SDL_UnlockMutex(SDL::GetMutex());
}


bool GStreamerVideo::Initialize()
{
    bool retVal = true;

    std::string path = Configuration::GetAbsolutePath() + "/Core";
    gst_init(NULL, NULL);

#ifdef WIN32
    GstRegistry *registry = gst_registry_get();
    gst_registry_scan_path(registry, path.c_str());
#endif

    Initialized = true;

    return retVal;
}

bool GStreamerVideo::DeInitialize()
{
    gst_deinit();
    Initialized = false;
    return true;
}


bool GStreamerVideo::Stop()
{
    if(!Initialized)
    {
        return false;
    }

    if(VideoSink)
    {
        g_object_set(G_OBJECT(VideoSink), "signal-handoffs", FALSE, NULL);
    }

    if(Playbin)
    {
        (void)gst_element_set_state(Playbin, GST_STATE_NULL);
    }

   // FreeElements();

    IsPlaying = false;
    Height = 0;
    Width = 0;
    FrameReady = false;

    return true;
}

bool GStreamerVideo::Play(std::string file)
{
    PlayCount = 0;

    if(!Initialized)
    {
        return false;
    }

    CurrentFile = file;

    const gchar *uriFile = gst_filename_to_uri (file.c_str(), NULL);
    if(!uriFile)
    {
        return false;
    }
    else
    {
        Configuration::ConvertToAbsolutePath(Configuration::GetAbsolutePath(), file);
        file = uriFile;

        if(!Playbin)
        {
            Playbin = gst_element_factory_make("playbin", "player");
            VideoBin = gst_bin_new("SinkBin");
            VideoSink  = gst_element_factory_make("fakesink", "video_sink");
            VideoConvert  = gst_element_factory_make("capsfilter", "video_convert");
            VideoConvertCaps = gst_caps_from_string("video/x-raw,format=(string)YUY2");
            Height = 0;
            Width = 0;
            if(!Playbin)
            {
                Logger::Write(Logger::ZONE_DEBUG, "Video", "Could not create playbin");
                FreeElements();
                return false;
            }
            if(!VideoSink)
            {
                Logger::Write(Logger::ZONE_DEBUG, "Video", "Could not create video sink");
                FreeElements();
                return false;
            }
            if(!VideoConvert)
            {
                Logger::Write(Logger::ZONE_DEBUG, "Video", "Could not create video converter");
                FreeElements();
                return false;
            }
            if(!VideoConvertCaps)
            {
                Logger::Write(Logger::ZONE_DEBUG, "Video", "Could not create video caps");
                FreeElements();
                return false;
            }

            gst_bin_add_many(GST_BIN(VideoBin), VideoConvert, VideoSink, NULL);
            gst_element_link_filtered(VideoConvert, VideoSink, VideoConvertCaps);
            GstPad *videoConvertSinkPad = gst_element_get_static_pad(VideoConvert, "sink");

            if(!videoConvertSinkPad)
            {
                Logger::Write(Logger::ZONE_DEBUG, "Video", "Could not get video convert sink pad");
                FreeElements();
                return false;
            }

            g_object_set(G_OBJECT(VideoSink), "sync", TRUE, "qos", FALSE, NULL);

            GstPad *videoSinkPad = gst_ghost_pad_new("sink", videoConvertSinkPad);
            if(!videoSinkPad)
            {
                Logger::Write(Logger::ZONE_DEBUG, "Video", "Could not get video bin sink pad");
                FreeElements();
                gst_object_unref(videoConvertSinkPad);
                videoConvertSinkPad = NULL;
                return false;
            }

            gst_element_add_pad(VideoBin, videoSinkPad);
            gst_object_unref(videoConvertSinkPad);
            videoConvertSinkPad = NULL;
        }
        g_object_set(G_OBJECT(Playbin), "uri", file.c_str(), "video-sink", VideoBin, NULL);

        IsPlaying = true;


        g_object_set(G_OBJECT(VideoSink), "signal-handoffs", TRUE, NULL);
        g_signal_connect(VideoSink, "handoff", G_CALLBACK(ProcessNewBuffer), this);

        VideoBus = gst_pipeline_get_bus(GST_PIPELINE(Playbin));
        gst_bus_add_watch(VideoBus, &BusCallback, this);

        /* Start playing */
        GstStateChangeReturn playState = gst_element_set_state(GST_ELEMENT(Playbin), GST_STATE_PLAYING);
        if (playState != GST_STATE_CHANGE_ASYNC)
        {
            IsPlaying = false;
            std::stringstream ss;
            ss << "Unable to set the pipeline to the playing state: ";
            ss << playState;
            Logger::Write(Logger::ZONE_ERROR, "Video", ss.str());
            FreeElements();
            return false;
        }
    }

    return true;
}

void GStreamerVideo::FreeElements()
{
    if(VideoBin)
    {
        gst_object_unref(VideoBin);
        VideoBin = NULL;
    }
    if(VideoSink)
    {
        gst_object_unref(VideoSink);
        VideoSink = NULL;
    }
    if(VideoConvert)
    {
        gst_object_unref(VideoConvert);
        VideoConvert = NULL;
    }
    if(VideoConvertCaps)
    {
        gst_object_unref(VideoConvertCaps);
        VideoConvertCaps = NULL;
    }
    if(Playbin)
    {
        gst_object_unref(Playbin);
        Playbin = NULL;
    }

}

void GStreamerVideo::Draw()
{
    FrameReady = false;
}

void GStreamerVideo::Update(float dt)
{
    SDL_LockMutex(SDL::GetMutex());
    if(!Texture && Width != 0 && Height != 0)
    {
        Texture = SDL_CreateTexture(SDL::GetRenderer(), SDL_PIXELFORMAT_YUY2,
                                    SDL_TEXTUREACCESS_STREAMING, Width, Height);
        SDL_SetTextureBlendMode(Texture, SDL_BLENDMODE_BLEND);
    }

    if(VideoBuffer && FrameReady && Texture && Width && Height)
    {
        //todo: change to width of cap
        void *pixels;
        int pitch;
        SDL_LockTexture(Texture, NULL, &pixels, &pitch);
        memcpy(pixels, VideoBuffer, Width*Height*2); //todo: magic number
        SDL_UnlockTexture(Texture);
    }
    SDL_UnlockMutex(SDL::GetMutex());


    if(VideoBus)
    {
        GstMessage *msg = gst_bus_pop(VideoBus);
        if(msg)
        {
            if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
            {
                PlayCount++;

                //todo: nesting hazard
                // if number of loops is 0, set to infinite (todo: this is misleading, rename variable)
                if(!NumLoops || NumLoops > PlayCount)
                {
                    gst_element_seek(Playbin,
                                     1.0,
                                     GST_FORMAT_TIME,
                                     GST_SEEK_FLAG_FLUSH,
                                     GST_SEEK_TYPE_SET,
                                     0,
                                     GST_SEEK_TYPE_NONE,
                                     GST_CLOCK_TIME_NONE);
                }
            }

            gst_message_unref(msg);
        }
    }
}

