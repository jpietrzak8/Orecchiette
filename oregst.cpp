//
// oregst.cpp
//
// Copyright 2013 by John Pietrzak  (jpietrzak8@gmail.com)
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

#include <QByteArray>
#include <QtDebug>
#include <QDate>

#include <glib.h>


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
  MainWindow *mw)
  : mainWindow(mw),
    myEncoding(AAC_Encoding),
    runningElement(0),
    paused(false),
    recordingPhone(false)
{
  gst_init (NULL,NULL);
  gst_version (&major, &minor, &micro, &nano);
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

  GstElement *encoder = getEncoder(filename);

  GstElement *outputFile = gst_element_factory_make("filesink", "outputFile");

  if (!outputFile)
  {
    throw OreException("Unable to create GStreamer element 'filesink'");
  }

  //qDebug() << "Recording phone to file: " << filename;
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
    encoder,
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

  if (!gst_element_link(combinedAudio, encoder))
  {
    throw OreException("Unable to link combinedAudio to encoder");
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

  GstElement *encoder = getEncoder(filename);

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

/*
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

  GstElement *encoder = getEncoder(filename);

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
*/

  GstElement *testsrc = gst_element_factory_make("audiotestsrc", "testsrc");

  if (!testsrc)
  {
    throw OreException("Unable to create GStreamer element 'audiotestsrc'");
  }

  GstElement *conv = gst_element_factory_make("audioconvert", "conv");

  if (!conv)
  {
    throw OreException("Unable to create GStreamer element 'audioconvert'");
  }

  GstElement *encoder = gst_element_factory_make("speexenc", "encoder");

  if (!encoder)
  {
    throw OreException("Unable to create GStreamer element 'speexenc'");
  }

  GstElement *outputFile = gst_element_factory_make("filesink", "outputFile");

  if (!outputFile)
  {
    throw OreException("Unable to create GStreamer element 'filesink'");
  }

  g_object_set(G_OBJECT(outputFile), "location", "/media/mmc1/Audio/wavtest.spx", NULL);

  GstElement *finalPipe = gst_pipeline_new("finalPipe");

  if (!finalPipe)
  {
    throw OreException("Unable to create GStreamer pipe");
  }

  gst_bin_add_many(
    GST_BIN(finalPipe),
    testsrc,
    conv,
    encoder,
    outputFile,
    NULL);

  if (!gst_element_link(testsrc, conv))
  {
    throw OreException("Unable to link testsrc to conv");
  }

  if (!gst_element_link(conv, encoder))
  {
    throw OreException("Unable to link conv to encoder");
  }

  if (!gst_element_link(encoder, outputFile))
  {
    throw OreException("Unable to link encoder to outputFile");
  }

  // Start the recording:
  gst_element_set_state(finalPipe, GST_STATE_PLAYING);

  setRunningElement(finalPipe);
}


void OreGst::startPlaying(
//  bool useBT,
  QString filename)
{
  if (runningElement)
  {
    throw OreException("Audio manager already in use.");
  }

  GstElement *player = gst_element_factory_make("playbin2", "player");

  if (!player)
  {
    throw OreException("Unable to create GStreamer element 'playbin2'");
  }

  // Construct a URI:
  QString uriFilename = "file://";
  uriFilename += filename;
  QByteArray ba = uriFilename.toAscii();
  g_object_set(G_OBJECT(player), "uri", ba.data(), NULL);

  gst_element_set_state(player, GST_STATE_PLAYING);
  setRunningElement(player);

/*
  GstElement *fileSource =
    gst_element_factory_make("filesrc", "fileSource");

  if (!fileSource)
  {
    throw OreException("Unable to create GStreamer element 'filesrc'");
  }

  QByteArray ba = filename.toAscii();
  g_object_set(G_OBJECT(fileSource), "location", ba.data(), NULL);

  GstElement *decoder =
    gst_element_factory_make("speexdec", "decoder");

  if (!decoder)
  {
    throw OreException("Unable to create GStreamer element 'speexdec'");
  }

  GstElement *speakerSink =
    gst_element_factory_make("autoaudiosink", "speakerSink");

  if (!speakerSink)
  {
    throw OreException("Unable to create GSTreamer element 'autoaudiosink'");
  }

  GstElement *finalPipe = gst_pipeline_new("finalPipe");

  if (!finalPipe)
  {
    throw OreException("Unable to create GStreamer pipe.");
  }

  gst_bin_add_many(
    GST_BIN(finalPipe),
    fileSource,
    decoder,
    speakerSink,
    NULL);

  if (!gst_element_link(fileSource, decoder))
  {
    throw OreException("Unable to link fileSource to decoder");
  }

  if (!gst_element_link(decoder, speakerSink))
  {
    throw OreException("Unable to link decoder to speakerSink");
  }

  gst_element_set_state(finalPipe, GST_STATE_PLAYING);

  setRunningElement(finalPipe);
*/
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

    g_object_set(G_OBJECT(enc), "bitrate", 128000, NULL);
    g_object_set(G_OBJECT(enc), "output-format", 1, NULL);

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
