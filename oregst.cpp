//
// oregst.cpp
//
// Copyright 2013 - 2015 by John Pietrzak  (jpietrzak8@gmail.com)
//
// This file is part of Orecchette.
//
// Orecchette is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Orecchette is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Orecchette; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "oregst.h"

#include "oreexception.h"
#include "mainwindow.h"
#include "orepreferencesform.h"

#include <QByteArray>
#include <QtDebug>
#include <QDate>
#include <QApplication>

#include <gst/interfaces/xoverlay.h>

#include <glib.h>

#include <libplayback/playback.h>


// In order to receive messages from GStreamer, we need a callback:
static gboolean oreGstBusCallback(
  GstBus *bus,
  GstMessage *msg,
  gpointer oreGstObjectPtr)
{
  Q_UNUSED(bus);

  switch (GST_MESSAGE_TYPE (msg))
  {
  case GST_MESSAGE_EOS:
    {
      OreGst *myGst = static_cast<OreGst *>(oreGstObjectPtr);
      myGst->stopCurrentElement();
    }
    break;

/*
  case GST_MESSAGE_STATE_CHANGED:
    {
      GstState old_state, new_state;
 
      gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);

      qDebug() << "[" << GST_OBJECT_NAME (msg->src) << "]: "
               << gst_element_state_get_name(old_state) << " -> "
               << gst_element_state_get_name(new_state);
    }
    break;
*/

/*
  // This is going to take some work:
  case GST_MESSAGE_TAG:
    {
      GstTagList *taglist = NULL;

      gst_message_parse_tag(msg, &taglist);
    }
*/

  case GST_MESSAGE_ERROR:
    {
      gchar *debug;
      GError *err;
 
      gst_message_parse_error (msg, &err, &debug);
      QString errString = "[";
      errString += GST_OBJECT_NAME(msg->src);
      errString += "]: ";
      errString += err->message;
      errString += " ";
      errString += debug;
      qDebug() << errString;
      OreException e(errString);
      e.display();
      OreGst *myGst = static_cast<OreGst *>(oreGstObjectPtr);
      myGst->stopCurrentElement();

      g_free (debug);
      g_error_free (err);
    }
    break;

  default:
/*
    {
      const GstStructure *structure = msg->structure;
      if (structure)
      {
        QString serialized = QString("%1{%2}: ")
          .arg(gst_message_type_get_name (msg->type))
          .arg(gst_structure_get_name(structure));

        for (int i = 0; i < gst_structure_n_fields(structure); ++i)
        {
          if (i != 0) serialized.append(", ");

          const char *name = gst_structure_nth_field_name(structure, i);
          GType type = gst_structure_get_field_type(structure, name);
          serialized.append(name);
          serialized.append(QString("[%1]").arg(g_type_name(type)));
        }

        qDebug () << serialized;

      }
      else
      {
        qDebug ("%s{}", gst_message_type_get_name (msg->type));
      }
    }
*/
    break;
  }
  return true;
}


//
// Now, on to the actual class methods:
//

OreGst::OreGst(
  MainWindow *mw,
  unsigned long vmwId)
  : mainWindow(mw),
    videoMonitorWindowId(vmwId),
    myEncoding(AAC_Encoding),
    tee(0),
    tee_colorspace1_pad(0),
    tee_colorspace2_pad(0),
    runningElement(0),
    paused(false),
    recordingPhone(false)
{
  gst_init (NULL,NULL);
}


OreGst::~OreGst()
{
  stopCurrentElement();
}


void OreGst::setAudioEncoding(
  AudioEncoding ae)
{
  myEncoding = ae;
}


void OreGst::startRecordingCall(
  const OrePreferencesForm &prefs,
  bool useBT,
  QString filename)
{
  if (runningElement)
  {
    throw OreException("Audio manager already in use.");
  }

  GstElement *microphoneSource = 0;
    gst_element_factory_make("pulsesrc", "microphoneSource");

  if (!microphoneSource)
  {
    throw OreException("Unable to create Gstreamer element 'pulsesrc'");
  }

  if (useBT)
  {
    g_object_set(G_OBJECT(microphoneSource), "device", "source.hw1", NULL);
  }
  else
  {
    g_object_set(G_OBJECT(microphoneSource), "device", "source.voice", NULL);
  }

  GstElement *speakerSource =
    gst_element_factory_make("pulsesrc", "speakerSource");

  if (!speakerSource)
  {
    throw OreException("Unable to create GStreamer element 'pulsesrc'");
  }

  if (useBT)
  {
    g_object_set(G_OBJECT(speakerSource), "device", "sink.hw1.monitor", NULL);
  }
  else
  {
    g_object_set(G_OBJECT(speakerSource), "device", "sink.hw0.monitor", NULL);
//    g_object_set(G_OBJECT(speakerSource), "device", "sink.voice", NULL);
  }

  GstElement *audioAdder =
    gst_element_factory_make("adder", "audioAdder");

  if (!audioAdder)
  {
    throw OreException("Unable to create GStreamer element 'adder'");
  }

  GstElement *finalPipe = gst_pipeline_new("finalPipe");

  if (!finalPipe)
  {
    throw OreException("Unable to create GStreamer pipe");
  }

  // Construct the combined audio source:
  switch (prefs.getAudioPosition())
  {
  case Mic_Left:
    {

      GstElement *micPan =
        gst_element_factory_make("audiopanorama", "micPan");

      if (!micPan)
      {
        throw OreException("Unable to create GStreamer element 'audiopanorama'");
      }

     g_object_set(
       G_OBJECT(micPan),
       "panorama", -1.0,
       NULL);

      GstElement *speakerPan =
        gst_element_factory_make("audiopanorama", "speakerPan");

      if (!speakerPan)
      {
        throw OreException("Unable to create GStreamer element 'audiopanorama'");
      }

     g_object_set(
       G_OBJECT(speakerPan),
       "panorama", 1.0,
       NULL);

      gst_bin_add_many(
        GST_BIN(finalPipe),
        microphoneSource,
        micPan,
        speakerSource,
        speakerPan,
        audioAdder,
        NULL);

      if (!gst_element_link(microphoneSource, micPan))
      {
        throw OreException("Unable to link microphoneSource to micPan");
      }

      if (!gst_element_link(micPan, audioAdder))
      {
        throw OreException("Unable to link micPan to audioAdder");
      }

      if (!gst_element_link(speakerSource, speakerPan))
      {
        throw OreException("Unable to link speakerSource to speakerPan");
      }

      if (!gst_element_link(speakerPan, audioAdder))
      {
        throw OreException("Unable to link speakerPan to audioAdder");
      }
    }

    break;

  case Mic_Right:
    {
      GstElement *micPan =
        gst_element_factory_make("audiopanorama", "micPan");

      if (!micPan)
      {
        throw OreException("Unable to create GStreamer element 'audiopanorama'");
      }

     g_object_set(
       G_OBJECT(micPan),
       "panorama", 1.0,
       NULL);

      GstElement *speakerPan =
        gst_element_factory_make("audiopanorama", "speakerPan");

      if (!speakerPan)
      {
        throw OreException("Unable to create GStreamer element 'audiopanorama'");
      }

     g_object_set(
       G_OBJECT(speakerPan),
       "panorama", -1.0,
       NULL);

      gst_bin_add_many(
        GST_BIN(finalPipe),
        microphoneSource,
        micPan,
        speakerSource,
        speakerPan,
        audioAdder,
        NULL);

      if (!gst_element_link(microphoneSource, micPan))
      {
        throw OreException("Unable to link microphoneSource to micPan");
      }

      if (!gst_element_link(micPan, audioAdder))
      {
        throw OreException("Unable to link micPan to audioAdder");
      }

      if (!gst_element_link(speakerSource, speakerPan))
      {
        throw OreException("Unable to link speakerSource to speakerPan");
      }

      if (!gst_element_link(speakerPan, audioAdder))
      {
        throw OreException("Unable to link speakerPan to audioAdder");
      }
    }

    break;

  case Mic_Center:
  default:
    {
      gst_bin_add_many(
        GST_BIN(finalPipe),
        microphoneSource,
        speakerSource,
        audioAdder,
        NULL);

      if (!gst_element_link(microphoneSource, audioAdder))
      {
        throw OreException("Unable to link microphoneSource to audioAdder");
      }

      if (!gst_element_link(speakerSource, audioAdder))
      {
        throw OreException("Unable to link speakerSource to audioAdder");
      }
    }

    break;
  }

  GstElement *encoder = getEncoder(prefs, filename);

  GstElement *outputFile = gst_element_factory_make("filesink", "outputFile");

  if (!outputFile)
  {
    throw OreException("Unable to create GStreamer element 'filesink'");
  }

  //qDebug() << "Recording phone to file: " << filename;
  QByteArray ba = filename.toAscii();
  g_object_set(G_OBJECT(outputFile), "location", ba.data(), NULL);

  gst_bin_add_many(
    GST_BIN(finalPipe),
    audioAdder,
    encoder,
    outputFile,
    NULL);

  if (!gst_element_link(audioAdder, encoder))
  {
    throw OreException("Unable to link audioAdder to encoder");
  }

  if (!gst_element_link(encoder, outputFile))
  {
    throw OreException("Unable to link encoder to outputFile");
  }

  // Start the recording:
  gst_element_set_state(finalPipe, GST_STATE_PLAYING);

  setRunningElement(finalPipe);

  recordingPhone = true;
}


/*
void OreGst::startRecordingMicrophone(
  bool useBT,
  QString filename)
{
  if (runningElement)
  {
    throw OreException("Audio manager already in use.");
  }

  GstElement *microphoneSource =
    gst_element_factory_make("pulsesrc", "microphoneSource");

  if (!microphoneSource)
  {
    throw OreException("Unable to create GStreamer element 'pulsesrc'");
  }

  if (useBT)
  {
    g_object_set(G_OBJECT(microphoneSource), "device", "source.hw1", NULL);
  }
  else
  {
    g_object_set(G_OBJECT(microphoneSource), "device", "source.voice", NULL);
  }

  GstElement *encoder = getEncoder(prefs, filename);

  GstElement *outputFile = gst_element_factory_make("filesink", "outputFile");

  if (!outputFile)
  {
    throw OreException("Unable to create GStreamer element 'filesink'");
  }

  //qDebug() << "Recording to file: " << filename;
  QByteArray ba = filename.toAscii();
  g_object_set(G_OBJECT(outputFile), "location", ba.data(), NULL);

  GstElement *finalPipe = gst_pipeline_new("finalPipe");

  if (!finalPipe)
  {
    throw OreException("Unable to create GStreamer pipe");
  }

  gst_bin_add_many(
    GST_BIN(finalPipe),
    microphoneSource,
    encoder,
    outputFile,
    NULL);

  if (!gst_element_link(microphoneSource, encoder))
  {
    throw OreException("Unable to link microphoneSource to encoder");
  }

  if (!gst_element_link(encoder, outputFile))
  {
    throw OreException("Unable to link encoder to outputFile");
  }

  // Start the recording:
  gst_element_set_state(finalPipe, GST_STATE_PLAYING);

  setRunningElement(finalPipe);
}


void OreGst::startRecordingSpeaker(
  bool useBT,
  QString filename)
{
  if (runningElement)
  {
    throw OreException("Audio manager already in use.");
  }

  GstElement *speakerSource =
    gst_element_factory_make("pulsesrc", "speakerSource");

  if (!speakerSource)
  {
    throw OreException("Unable to create GStreamer element 'pulsesrc'");
  }

  if (useBT)
  {
    g_object_set(G_OBJECT(speakerSource), "device", "sink.hw1.monitor", NULL);
  }
  else
  {
    g_object_set(G_OBJECT(speakerSource), "device", "sink.hw0.monitor", NULL);
  }

  GstElement *encoder = getEncoder(prefs, filename);

  GstElement *outputFile = gst_element_factory_make("filesink", "outputFile");

  if (!outputFile)
  {
    throw OreException("Unable to create GStreamer element 'filesink'");
  }

  QByteArray ba = filename.toAscii();
  g_object_set(G_OBJECT(outputFile), "location", ba.data(), NULL);

  GstElement *finalPipe = gst_pipeline_new("finalPipe");

  if (!finalPipe)
  {
    throw OreException("Unable to create GStreamer pipe");
  }

  gst_bin_add_many(
    GST_BIN(finalPipe),
    speakerSource,
    encoder,
    outputFile,
    NULL);

  if (!gst_element_link(speakerSource, encoder))
  {
    throw OreException("Unable to link speakerSource to encoder");
  }

  if (!gst_element_link(encoder, outputFile))
  {
    throw OreException("Unable to link encoder to outputFile");
  }

  // Start the recording:
  gst_element_set_state(finalPipe, GST_STATE_PLAYING);

  setRunningElement(finalPipe);
}


// Ultimate combo recording: microphone, speaker, and screen!
void OreGst::startRecordingScreen(
  bool useBT,
  QString filename)
{
  if (runningElement)
  {
    throw OreException("GStreamer already in use.");
  }

  GstElement *microphoneSource =
    gst_element_factory_make("pulsesrc", "microphoneSource");

  if (!microphoneSource)
  {
    throw OreException("Unable to create Gstreamer element 'pulsesrc'");
  }

  if (useBT)
  {
    g_object_set(G_OBJECT(microphoneSource), "device", "source.hw1", NULL);
  }
  else
  {
    g_object_set(G_OBJECT(microphoneSource), "device", "source.voice", NULL);
  }

  GstElement *speakerSource =
    gst_element_factory_make("pulsesrc", "speakerSource");

  if (!speakerSource)
  {
    throw OreException("Unable to create GStreamer element 'pulsesrc'");
  }

  if (useBT)
  {
    g_object_set(G_OBJECT(speakerSource), "device", "sink.hw1.monitor", NULL);
  }
  else
  {
    g_object_set(G_OBJECT(speakerSource), "device", "sink.hw0.monitor", NULL);
  }

  GstElement *combinedAudio =
    gst_element_factory_make("adder", "combinedAudio");

  if (!combinedAudio)
  {
    throw OreException("Unable to create GStreamer element 'adder'");
  }

//  GstElement *encoder = getEncoder(prefs, filename);

  GstElement *audioConverter =
    gst_element_factory_make("audioconvert", "audioConverter");

  if (!audioConverter)
  {
    throw OreException("Unable to create GStreamer element 'audioconvert'");
  }

  GstElement *videoSource =
    gst_element_factory_make("ximagesrc", "videoSource");

  if (!videoSource)
  {
    throw OreException("Unable to create GStreamer element 'ximagesrc'");
  }

  g_object_set(
    G_OBJECT(videoSource),
    "caps", 
    gst_caps_new_simple(
      "video/x-raw-rgb",
//      "format", G_TYPE_STRING, "RGB16",
//      "width", G_TYPE_INT, 800,
//      "height", G_TYPE_INT, 480,
      "framerate", GST_TYPE_FRACTION, "5", "1",
      NULL),
    NULL);

  GstElement *someMpegColorThing =
    gst_element_factory_make("ffmpegcolorspace", "someMpegColorThing");

  if (!someMpegColorThing)
  {
    throw OreException("Unable to create GStreamer element 'ffmpegcolorspace'");
  }

  GstElement *videoEncoder =
    gst_element_factory_make("dspmp4venc", "videoEncoder");

  if (!videoEncoder)
  {
    throw OreException("Unable to create GStreamer element 'dspmp4venc'");
  }

//  g_object_set(G_OBJECT(videoEncoder), "quality", 30, NULL);

  GstElement *videoQueue =
    gst_element_factory_make("queue", "videoQueue");

  if (!videoQueue)
  {
    throw OreException("Unable to create GStreamer element 'queue'");
  }
 
  GstElement *avContainer =
    gst_element_factory_make("matroskamux", "avcontainer");

  if (!avContainer)
  {
    throw OreException("Unable to create GStreamer element 'matroskamux'");
  }

  GstElement *outputFile = gst_element_factory_make("filesink", "outputFile");

  if (!outputFile)
  {
    throw OreException("Unable to create GStreamer element 'filesink'");
  }

  qDebug() << "Recording video to file: " << filename;
  QByteArray ba = filename.toAscii();
  g_object_set(G_OBJECT(outputFile), "location", ba.data(), NULL);

  GstElement *finalPipe = gst_pipeline_new("finalPipe");

  if (!finalPipe)
  {
    throw OreException("Unable to create GStreamer pipe");
  }

  gst_bin_add_many(
    GST_BIN(finalPipe),
    microphoneSource,
    speakerSource,
    combinedAudio,
//    encoder,
    audioConverter,
    videoSource,
    someMpegColorThing,
    videoEncoder,
    videoQueue,
    avContainer,
    outputFile,
    NULL);

  if (!gst_element_link(microphoneSource, combinedAudio))
  {
    throw OreException("Unable to link microphoneSource to combinedAudio");
  }

  if (!gst_element_link(speakerSource, combinedAudio))
  {
    throw OreException("Unable to link speakerSource to combinedAudio");
  }

  if (!gst_element_link(combinedAudio, audioConverter))
  {
    throw OreException("Unable to link combinedAudio to audioConverter");
  }

  if (!gst_element_link(audioConverter, avContainer))
  {
    throw OreException("Unable to link audioConverter to avContainer");
  }

  if (!gst_element_link(videoSource, someMpegColorThing))
  {
    throw OreException("Unable to link videoSource to someMpegColorThing");
  }

  if (!gst_element_link(someMpegColorThing, videoEncoder))
  {
    throw OreException("Unable to link someMpegColorThing to videoEncoder");
  }

  if (!gst_element_link(videoEncoder, videoQueue))
  {
    throw OreException("Unable to link videoEncoder to videoQueue");
  }

  if (!gst_element_link(videoQueue, avContainer))
  {
    throw OreException("Unable to link videoQueue to avContainer");
  }

  if (!gst_element_link(avContainer, outputFile))
  {
    throw OreException("Unable to link avContainer to outputFile");
  }

  // Start the recording:
  gst_element_set_state(finalPipe, GST_STATE_PLAYING);

  setRunningElement(finalPipe);
}
*/


void OreGst::startRecording(
  const OrePreferencesForm &prefs,
  bool useBT,
  QString filename,
  OreAudioSource audioChoice,
  OreVideoSource videoChoice)
{
  if (runningElement)
  {
    throw OreException("GStreamer manager currently busy.");
  }

  // Sanity check:
  if ((audioChoice == No_Audio) && (videoChoice == No_Video))
  {
    // Nothing to do.
    return;
  }

  // All the various elements will eventually be added to the final pipe:
  GstElement *finalPipe = gst_pipeline_new("finalPipe");

  if (!finalPipe)
  {
    throw OreException("Unable to create GStreamer pipe");
  }

  // Now, start creating the various elements:
  GstElement *audioSrcElement = 0;

  switch (audioChoice)
  {
  case Microphone_Audio:
    {
      audioSrcElement =
        gst_element_factory_make("pulsesrc", "microphoneSource");

      if (!audioSrcElement)
      {
        throw OreException("Unable to create GStreamer element 'pulsesrc'");
      }

      if (useBT)
      {
        g_object_set(G_OBJECT(audioSrcElement), "device", "source.hw1", NULL);
      }
      else
      {
        g_object_set(G_OBJECT(audioSrcElement), "device", "source.voice", NULL);
      }

      gst_bin_add(GST_BIN(finalPipe), audioSrcElement);
    }

    break;

  case Speaker_Audio:
    {
      audioSrcElement = 
        gst_element_factory_make("pulsesrc", "speakerSource");

      if (!audioSrcElement)
      {
        throw OreException("Unable to create GStreamer element 'pulsesrc'");
      }

      if (useBT)
      {
        g_object_set(
          G_OBJECT(audioSrcElement), "device", "sink.hw1.monitor", NULL);
      }
      else
      {
        g_object_set(G_OBJECT(audioSrcElement), "device", "sink.hw0.monitor", NULL);
//        g_object_set(G_OBJECT(audioSrcElement), "device", "sink.voice", NULL);
      }

      gst_bin_add(GST_BIN(finalPipe), audioSrcElement);
    }

    break;

  case MicrophoneAndSpeaker_Audio:
    {
      GstElement *microphoneSource =
        gst_element_factory_make("pulsesrc", "microphoneSource");

      GstElement *speakerSource = 
        gst_element_factory_make("pulsesrc", "speakerSource");

      if (!microphoneSource || !speakerSource)
      {
        throw OreException("Unable to create GStreamer element 'pulsesrc'");
      }

      if (useBT)
      {
        g_object_set(G_OBJECT(microphoneSource), "device", "source.hw1", NULL);
        g_object_set(
          G_OBJECT(speakerSource), "device", "sink.hw1.monitor", NULL);
      }
      else
      {
        g_object_set(
          G_OBJECT(microphoneSource), "device", "source.voice", NULL);
        g_object_set(
          G_OBJECT(speakerSource), "device", "sink.hw0.monitor", NULL);
      }

      audioSrcElement = 
        gst_element_factory_make("adder", "combinedAudio");

      if (!audioSrcElement)
      {
        throw OreException("Unable to create GStreamer element 'adder'");
      }

      switch (prefs.getAudioPosition())
      {
      case Mic_Left:
        {
          GstElement *micPan =
            gst_element_factory_make("audiopanorama", "micPan");

          if (!micPan)
          {
            throw OreException(
              "Unable to create GStreamer element 'audiopanorama'");
          }

          g_object_set(
            G_OBJECT(micPan),
            "panorama", -1.0,
            NULL);

          GstElement *speakerPan =
            gst_element_factory_make("audiopanorama", "speakerPan");

          if (!speakerPan)
          {
            throw OreException(
             "Unable to create GStreamer element 'audiopanorama'");
          }

         g_object_set(
           G_OBJECT(speakerPan),
           "panorama", 1.0,
           NULL);

          gst_bin_add_many(
            GST_BIN(finalPipe),
            microphoneSource,
            micPan,
            speakerSource,
            speakerPan,
            audioSrcElement,
            NULL);

          if (!gst_element_link(microphoneSource, micPan))
          {
            throw OreException(
              "Unable to link microphoneSource to micPan");
          }

          if (!gst_element_link(micPan, audioSrcElement))
          {
            throw OreException(
              "Unable to link micPan to audioSrcElement");
          }

          if (!gst_element_link(speakerSource, speakerPan))
          {
            throw OreException(
              "Unable to link speakerSource to speakerPan");
          }

          if (!gst_element_link(speakerPan, audioSrcElement))
          {
            throw OreException(
              "Unable to link speakerPan to audioSrcElement");
          }
        }
        break;

      case Mic_Right:
        {
          GstElement *micPan =
            gst_element_factory_make("audiopanorama", "micPan");

          if (!micPan)
          {
            throw OreException(
              "Unable to create GStreamer element 'audiopanorama'");
          }

          g_object_set(
            G_OBJECT(micPan),
            "panorama", 1.0,
            NULL);

          GstElement *speakerPan =
            gst_element_factory_make("audiopanorama", "speakerPan");

          if (!speakerPan)
          {
            throw OreException(
             "Unable to create GStreamer element 'audiopanorama'");
          }

         g_object_set(
           G_OBJECT(speakerPan),
           "panorama", -1.0,
           NULL);

          gst_bin_add_many(
            GST_BIN(finalPipe),
            microphoneSource,
            micPan,
            speakerSource,
            speakerPan,
            audioSrcElement,
            NULL);

          if (!gst_element_link(microphoneSource, micPan))
          {
            throw OreException(
              "Unable to link microphoneSource to micPan");
          }

          if (!gst_element_link(micPan, audioSrcElement))
          {
            throw OreException(
              "Unable to link micPan to audioSrcElement");
          }

          if (!gst_element_link(speakerSource, speakerPan))
          {
            throw OreException(
              "Unable to link speakerSource to speakerPan");
          }

          if (!gst_element_link(speakerPan, audioSrcElement))
          {
            throw OreException(
              "Unable to link speakerPan to audioSrcElement");
          }
        }
        break;

      case Mic_Center:
      default:
        {
          gst_bin_add_many(
            GST_BIN(finalPipe),
            microphoneSource,
            speakerSource,
            audioSrcElement,
            NULL);

          if (!gst_element_link(microphoneSource, audioSrcElement))
          {
            throw OreException(
              "Unable to link microphoneSource to audioSrcElement");
          }

          if (!gst_element_link(speakerSource, audioSrcElement))
          {
            throw OreException(
              "Unable to link speakerSource to audioSrcElement");
          }
        }
        break;
      }

      // Not sure if I should be setting this here:
      recordingPhone = true;
    }

    break;

  case No_Audio:
  default:
    break;
  }

  // If we've created an audio source element, set up an audio encoder,
  // and add it to the pipe:
  GstElement *audioEncoder = 0;

  if (audioSrcElement)
  {
    // For straight audio, we can use the standard encoders.  But for video,
    // Matroska does not support AAC, so we have to switch to RAW.
    if (videoChoice == No_Video)
    {
      audioEncoder = getEncoder(prefs, filename);
    }
    else
    {
      // Skip the encoder, and setup an "audioconvert" element to make
      // everything work with Matroska.
      audioEncoder = gst_element_factory_make("audioconvert", "audioConverter");
    }

    gst_bin_add(GST_BIN(finalPipe), audioEncoder);

    if (!gst_element_link(audioSrcElement, audioEncoder))
    {
      throw OreException("Unable to link audioSrcElement to audioEncoder");
    }
  }

  // Video options:
  bool useMonitor = false;  // Only show the video monitor if using cameras.
  GstElement *videoSource = 0;

  switch (videoChoice)
  {
  case Screen_Video:
    {
      videoSource = gst_element_factory_make("ximagesrc", "videoSource");

      if (!videoSource)
      {
        throw OreException("Unable to create GStreamer element 'ximagesrc'");
      }

      g_object_set(
        G_OBJECT(videoSource),
        "caps", 
        gst_caps_new_simple(
          "video/x-raw-rgb",
//          "format", G_TYPE_STRING, "RGB16",
//          "width", G_TYPE_INT, 800,
//          "height", G_TYPE_INT, 480,
          "framerate", GST_TYPE_FRACTION, "5", "1",
          NULL),
        NULL);
    }

    break;

  case BackCamera_Video:
    {
      videoSource = gst_element_factory_make("v4l2camsrc", "videoSource");

      if (!videoSource)
      {
        throw OreException("Unable to create GStreamer element 'v4l2camsrc'");
      }

      g_object_set(
        G_OBJECT(videoSource),
        "device", "/dev/video0",
        "width", 320,
        "height", 240,
        NULL);

      useMonitor = true;
    }

    break;

  case FrontCamera_Video:
    {
      videoSource = gst_element_factory_make("v4l2camsrc", "videoSource");

      if (!videoSource)
      {
        throw OreException("Unable to create GStreamer element 'v4l2camsrc'");
      }

      g_object_set(
        G_OBJECT(videoSource),
        "device", "/dev/video1",
        "width", 320,
        "height", 240,
        NULL);

      useMonitor = true;
    }

    break;

/*
  case MJpegStream_Video:
    {
      GstElement *mjpegSrc =
        gst_element_factory_make("souphttpsrc", "mjpegSource");

      if (!mjpegSrc)
      {
        throw OreException("Unable to create GStreamer element 'souphttpsrc'");
      }

      g_object_set(
        G_OBJECT(mjpegSrc),
        "location", mjpegStreamUrl.constData(),
        "timeout", 10,
        NULL);

      GstElement *demux =
        gst_element_factory_make("multipartdemux", "demux");

      if (!demux)
      {
        throw OreException("Unable to create GStreamer element 'multipartdemux'");
      }

      videoSource = gst_element_factory_make("jpegdec", "videoSource");

      if (!videoSource)
      {
        throw OreException("Unable to create GStreamer element 'jpegdec'");
      }

      gst_bin_add_many(
        GST_BIN(finalPipe),
        mjpegSrc,
 //       demux,
        videoSource,
        NULL);

      if (!gst_element_link(mjpegSrc, videoSource))
      {
        throw OreException("Unable to link mjpegSrc to videoSource");
      }

      if (!gst_element_link(demux, videoSource))
      {
        throw OreException("Unable to link demux to videoSource");
      }

      useMonitor = true;
    }

    break;
*/

  case No_Video:
  default:
    break;
  }

  // If a video source is defined, set up associated support elements:
  GstElement *videoQueue = 0;
  if (videoSource)
  {
    if (useMonitor)
    {
      videoQueue = generateSplitPipe(videoSource, finalPipe);
    }
    else
    {
      videoQueue = generateLinearPipe(videoSource, finalPipe);
    }
  }

  // Finally, if we have video, dump both audio & video to a matroska
  // container.  Otherwise, just dump audio straight to a file.

  if (videoQueue)
  {
    GstElement *avContainer =
      gst_element_factory_make("matroskamux", "avcontainer");

    if (!avContainer)
    {
      throw OreException("Unable to create GStreamer element 'matroskamux'");
    }

    GstElement *outputFile =
      gst_element_factory_make("filesink", "outputFile");

    if (!outputFile)
    {
      throw OreException("Unable to create GStreamer element 'filesink'");
    }

qDebug() << "Recording video to file: " << filename;
    QByteArray ba = filename.toAscii();
    g_object_set(G_OBJECT(outputFile), "location", ba.data(), NULL);

    gst_bin_add_many(
      GST_BIN(finalPipe),
      avContainer,
      outputFile,
      NULL);

    // If we are encoding audio, link it to the container:
    if (audioEncoder)
    {
      if (!gst_element_link(audioEncoder, avContainer))
      {
        throw OreException("Unable to link audioEncoder to avContainer");
      }
    }

    // Link the video to the container:
    if (!gst_element_link(videoQueue, avContainer))
    {
      throw OreException("Unable to link videoQueue to avContainer");
    }

    // Finally, link the container to the file:
    if (!gst_element_link(avContainer, outputFile))
    {
      throw OreException("Unable to link avContainer to outputFile");
    }
  }
  else
  {
    GstElement *outputFile =
      gst_element_factory_make("filesink", "outputFile");

    if (!outputFile)
    {
      throw OreException("Unable to create GStreamer element 'filesink'");
    }

    QByteArray ba = filename.toAscii();
    g_object_set(G_OBJECT(outputFile), "location", ba.data(), NULL);

    gst_bin_add(GST_BIN(finalPipe), outputFile);

    if (!gst_element_link(audioEncoder, outputFile))
    {
      throw OreException("Unable to link audioEncoder to outputFile");
    }
  }

  // Start recording!
  gst_element_set_state(finalPipe, GST_STATE_PLAYING);

  setRunningElement(finalPipe);
}


void OreGst::startPlaying(
  bool setupVideo,
  QString filename)
{
  if (runningElement)
  {
    throw OreException("Audio manager already in use.");
  }

  GstElement *xvsink = 0;

  if (setupVideo)
  {
    xvsink = gst_element_factory_make("xvimagesink", "xvsink");

    if (!xvsink)
    {
      throw OreException(
        "Unable to create GStreamer element 'xvimagesink'");
    }

    QApplication::syncX();
    gst_x_overlay_set_xwindow_id(
      GST_X_OVERLAY(G_OBJECT(xvsink)),
      videoMonitorWindowId);

    g_object_set(G_OBJECT(xvsink), "force_aspect_ratio", true, (char*)NULL);
    gst_element_set_state(xvsink, GST_STATE_READY);
  }

  GstElement *player = gst_element_factory_make("playbin2", "player");

  if (!player)
  {
    throw OreException("Unable to create GStreamer element 'playbin2'");
  }

  // Construct a URI:
  QString uriFilename = "file://";
  uriFilename += filename;
qDebug() << "playing file from: " << filename;
  QByteArray ba = uriFilename.toAscii();
  g_object_set(
    G_OBJECT(player),
    "uri", ba.data(),
    NULL);

  if (setupVideo)
  {
    g_object_set(
      G_OBJECT(player),
      "video-sink", xvsink,
      NULL);
  }

  gst_element_set_state(player, GST_STATE_PLAYING);
  setRunningElement(player);
}


void OreGst::pauseOrContinue()
{
  if (!runningElement) return;

  if (paused)
  {
    // Continue paused element
    gst_element_set_state(runningElement, GST_STATE_PLAYING);
    paused = false;
    mainWindow->continueDisplay();
  }
  else
  {
    gst_element_set_state(runningElement, GST_STATE_PAUSED);
    paused = true;
    mainWindow->pauseDisplay();
  }
}


void OreGst::stopCurrentElement()
{
  if (!runningElement) return;

//  qDebug() << "Attempting to stop current element.";

  gst_element_set_state(runningElement, GST_STATE_NULL);

  gst_object_unref(GST_OBJECT(runningElement));
  runningElement = 0;

  if (tee)
  {
    gst_object_unref(tee);
    tee = 0;
  }

  if (tee_colorspace1_pad)
  {
    gst_object_unref(tee_colorspace1_pad);
    tee_colorspace1_pad = 0;
  }

  if (tee_colorspace2_pad)
  {
    gst_object_unref(tee_colorspace2_pad);
    tee_colorspace2_pad = 0;
  }

  recordingPhone = false;

  mainWindow->startNewStatus(Idle_Status);
}


bool OreGst::gstreamerInUse()
{
  return (runningElement != 0);
}


bool OreGst::currentlyRecordingCall()
{
  return recordingPhone;
}


GstElement *OreGst::getEncoder(
  const OrePreferencesForm &prefs,
  QString filename)
{
  GstElement *enc;

  switch (myEncoding)
  {
  case FLAC_Encoding:
    enc = gst_element_factory_make("flacenc", "flac");

    if (!enc)
    {
      throw OreException("Unable to create GStreamer element 'flacenc'");
    }

    break;

  case WAV_Encoding:
    enc = gst_element_factory_make("wavenc", "wav");

    if (!enc)
    {
      throw OreException("Unable to create GStreamer element 'wavenc'");
    }

    break;

  case AAC_Encoding:
    enc = gst_element_factory_make("nokiaaacenc", "aac");

    if (!enc)
    {
      throw OreException("Unable to create GStreamer element 'nokiaaacenc'");
    }

    if (prefs.aacBitrateSet())
    {
      g_object_set(G_OBJECT(enc), "bitrate", prefs.aacBitrateValue(), NULL);
    }

    if (prefs.aacOutputFormatSet())
    {
      g_object_set(
        G_OBJECT(enc), "output-format", prefs.aacOutputFormatValue(), NULL);
    }

    if (prefs.aacWidthSet())
    {
      g_object_set(
        G_OBJECT(enc), "width", prefs.aacWidthValue(), NULL);
    }

    if (prefs.aacDepthSet())
    {
      g_object_set(
        G_OBJECT(enc), "depth", prefs.aacDepthValue(), NULL);
    }

    if (prefs.aacRateSet())
    {
      g_object_set(
        G_OBJECT(enc), "rate", prefs.aacRateValue(), NULL);
    }

    if (prefs.aacChannelsSet())
    {
      g_object_set(
        G_OBJECT(enc), "channels", prefs.aacChannelsValue(), NULL);
    }

    break;

  case ILBC_Encoding:
    enc = gst_element_factory_make("nokiailbcenc", "ilbc");

    if (!enc)
    {
      throw OreException("Unable to create GStreamer element 'nokiailbcenc'");
    }

    break;

  case SPX_Encoding:
  default:
    enc = gst_element_factory_make("speexenc", "spx");

    if (!enc)
    {
      throw OreException("Unable to create GStreamer element 'speexenc'");
    }

//    g_object_set(G_OBJECT(enc), "mode", "nb", NULL);

    break;
  }

/*
  GValue gvalue;
  g_value_init(&gvalue, G_TYPE_STRING);
  QByteArray ba = filename.toAscii();
  g_value_set_string(&gvalue, ba.data());

  gst_tag_setter_add_tag_value(
    GST_TAG_SETTER(enc), GST_TAG_MERGE_REPLACE,
    GST_TAG_TITLE, &gvalue);

  g_value_unset(&gvalue);

  g_value_set_static_string(&gvalue, "Orecchiette Recordings");

  gst_tag_setter_add_tag_value(
    GST_TAG_SETTER(enc), GST_TAG_MERGE_REPLACE,
    GST_TAG_ALBUM, &gvalue);

  g_value_unset(&gvalue);

  GDate *gdate = g_date_new();
  g_date_set_julian(gdate, QDate::currentDate().toJulianDay());
  g_value_init(&gvalue, G_TYPE_DATE);
  gst_value_set_date(&gvalue, gdate);

  gst_tag_setter_add_tag_value(
    GST_TAG_SETTER(enc), GST_TAG_MERGE_REPLACE,
    GST_TAG_DATE, &gvalue);

  g_value_unset(&gvalue);
  g_date_free(gdate);
*/

  return enc;
}


void OreGst::setRunningElement(
  GstElement *element)
{
  runningElement = element;

  GstBus *bus = gst_element_get_bus (GST_ELEMENT (element));
  gst_bus_add_watch (bus, oreGstBusCallback, this);
  gst_object_unref (bus);
}


GstElement *OreGst::generateLinearPipe(
  GstElement *videoSource,
  GstElement *finalPipe)
{
  GstElement *colorspace =
    gst_element_factory_make("ffmpegcolorspace", "colorspace");

  if (!colorspace)
  {
    throw OreException(
      "Unable to create GStreamer element 'ffmpegcolorspace'");
  }

  GstElement *videoEncoder =
    gst_element_factory_make("dspmp4venc", "videoEncoder");

  if (!videoEncoder)
  {
    throw OreException("Unable to create GStreamer element 'dspmp4venc'");
  }

  GstElement *videoQueue = 
    gst_element_factory_make("queue", "videoQueue");

  if (!videoQueue)
  {
    throw OreException("Unable to create GStreamer element 'queue'");
  }

  gst_bin_add_many(
    GST_BIN(finalPipe),
    videoSource,
    colorspace,
    videoEncoder,
    videoQueue,
    NULL);

  if (!gst_element_link(videoSource, colorspace))
  {
    throw OreException("Unable to link videoSource to colorspace");
  }

  if (!gst_element_link(colorspace, videoEncoder))
  {
    throw OreException("Unable to link colorspace to videoEncoder");
  }

  if (!gst_element_link(videoEncoder, videoQueue))
  {
    throw OreException("Unable to link videoEncoder to videoQueue");
  }

  return videoQueue;
}


GstElement *OreGst::generateSplitPipe(
  GstElement *videoSource,
  GstElement *finalPipe)
{
  GstElement *colorspace1 =
    gst_element_factory_make("ffmpegcolorspace", "colorspace1");

  if (!colorspace1)
  {
    throw OreException(
      "Unable to create GStreamer element 'ffmpegcolorspace'");
  }

  tee = gst_element_factory_make("tee", "tee");

  if (!tee)
  {
    throw OreException(
      "Unable to create GStreamer element 'tee'");
  }

  GstElement *scale = gst_element_factory_make("videoscale", "scale");

  if (!scale)
  {
    throw OreException(
      "Unable to create GStreamer element 'videoscale'");
  }

  GstElement *xvsink = gst_element_factory_make("xvimagesink", "xvsink");

  if (!xvsink)
  {
    throw OreException(
      "Unable to create GStreamer element 'xvimagesink'");
  }

  QApplication::syncX();
  gst_x_overlay_set_xwindow_id(
    GST_X_OVERLAY(G_OBJECT(xvsink)),
    videoMonitorWindowId);

  g_object_set(G_OBJECT(xvsink), "force_aspect_ratio", true, (char*)NULL);

  gst_element_set_state(xvsink, GST_STATE_READY);

  GstElement *colorspace2 =
    gst_element_factory_make("ffmpegcolorspace", "colorspace2");

  if (!colorspace2)
  {
    throw OreException(
      "Unable to create GStreamer element 'ffmpegcolorspace'");
  }

  GstElement *videoEncoder =
    gst_element_factory_make("dspmp4venc", "videoEncoder");

  if (!videoEncoder)
  {
    throw OreException("Unable to create GStreamer element 'dspmp4venc'");
  }

  GstElement *videoQueue = 
    gst_element_factory_make("queue", "videoQueue");

  if (!videoQueue)
  {
    throw OreException("Unable to create GStreamer element 'queue'");
  }

  gst_bin_add_many(
    GST_BIN(finalPipe),
    videoSource,
    tee,
    colorspace1,
    scale,
    xvsink,
    colorspace2,
    videoEncoder,
    videoQueue,
    NULL);

  if (!gst_element_link(videoSource, tee))
  {
    throw OreException("Unable to link videoSource to tee");
  }

  if (!gst_element_link(colorspace1, scale))
  {
    throw OreException("Unable to link colorspace1 to scale");
  }

  if (!gst_element_link(scale, xvsink))
  {
    throw OreException("Unable to link scale to xvsink");
  }

  if (!gst_element_link(colorspace2, videoEncoder))
  {
    throw OreException("Unable to link colorspace2 to videoEncoder");
  }

  if (!gst_element_link(videoEncoder, videoQueue))
  {
    throw OreException("Unable to link videoEncoder to videoQueue");
  }

  // Now, set up the tee:
  tee_colorspace1_pad = gst_element_get_request_pad(tee, "src%d");
  GstPad *colorspace1_pad = gst_element_get_static_pad(colorspace1, "sink");
  tee_colorspace2_pad = gst_element_get_request_pad(tee, "src%d");
  GstPad *colorspace2_pad = gst_element_get_static_pad(colorspace2, "sink");

  if (gst_pad_link(tee_colorspace1_pad, colorspace1_pad) != GST_PAD_LINK_OK
    || gst_pad_link(tee_colorspace2_pad, colorspace2_pad) != GST_PAD_LINK_OK)
  {
    throw OreException("Unable to set up tee");
  }

  gst_object_unref(colorspace1_pad);
  gst_object_unref(colorspace2_pad);

  return videoQueue;
}

