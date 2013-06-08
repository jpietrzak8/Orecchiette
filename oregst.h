//
// oregst.h
//
// Copyright 2013 by John Pietrzak  (jpietrzak8@gmail.com)
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

#include <QString>

class MainWindow;

class OreGst
{
public:
  OreGst(
    MainWindow *mw);
  ~OreGst();

  void setAudioEncoding(
    AudioEncoding ae);

  void startRecordingCall(
    bool useBT,
    QString filename);

  void startRecordingMicrophone(
    bool useBT,
    QString filename);

  void startRecordingSpeaker(
    bool useBT,
    QString filename);

  void startPlaying(
//    bool useBT,
    QString filename);

  void pauseOrContinue();

  void stopCurrentElement();

  bool gstreamerInUse();

  bool currentlyRecordingCall();

private:
  GstElement *getAdderPipe();

  GstElement *getEncoding(
    QString filename);

  void setRunningElement(
    GstElement *element);

  MainWindow *mainWindow;

  AudioEncoding myEncoding;

  GstElement *runningElement;

  bool paused;
  bool recordingPhone;

  guint major;
  guint minor;
  guint micro;
  guint nano; // 0 is release, 1 is from scm, 2 is prerelease
};

#endif // OREGST_H
