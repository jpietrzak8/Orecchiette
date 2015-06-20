//
// oregst.h
//
// Copyright 2013 - 2015 by John Pietrzak  (jpietrzak8@gmail.com)
//
// This file is part of Orecchiette.
//
// Orecchiette is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Orecchiette is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Orecchiette; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef OREGST_H
#define OREGST_H

#include <gst/gst.h>

#include "oreencoding.h"

#include <QObject>
#include <QString>

class MainWindow;
class OrePreferencesForm;

class OreGst: public QObject
{
Q_OBJECT

public:
  OreGst(
    MainWindow *mw,
    unsigned long vmwId);

  ~OreGst();

  void startRecordingCall(
    const OrePreferencesForm &prefs,
    bool useBT,
    QString filename);

/*
  void startRecordingMicrophone(
    bool useBT,
    QString filename);

  void startRecordingSpeaker(
    bool useBT,
    QString filename);

  void startRecordingScreen(
    bool useBT,
    QString filename);
*/

  void startRecording(
    const OrePreferencesForm &prefs,
    bool useBT,
    QString filename,
    OreAudioSource audioChoice,
    OreVideoSource videoChoice);

  void startPlaying(
    bool setupVideo,
    QString filename);

  void pauseOrContinue();

  void stopCurrentElement();

  bool gstreamerInUse();

  bool currentlyRecordingCall();

/*
  void setMJpegStreamUrl(
    QString url)
    {
      mjpegStreamUrl = url.toAscii();
    }
*/

public slots:
  void setAudioEncoding(
    AudioEncoding ae);

private:
  GstElement *getAdderPipe();

  GstElement *getEncoder(
    const OrePreferencesForm &prefs,
    QString filename);

  void setRunningElement(
    GstElement *element);

  GstElement *generateLinearPipe(
    GstElement *videoSource,
    GstElement *finalPipe);

  GstElement *generateSplitPipe(
    GstElement *videoSource,
    GstElement *finalPipe);

  MainWindow *mainWindow;

  unsigned long videoMonitorWindowId;

  AudioEncoding myEncoding;

  GstElement *tee;
  GstPad *tee_colorspace1_pad;
  GstPad *tee_colorspace2_pad;

  GstElement *runningElement;

  bool paused;
  bool recordingPhone;

//  QByteArray mjpegStreamUrl;
};

#endif // OREGST_H
