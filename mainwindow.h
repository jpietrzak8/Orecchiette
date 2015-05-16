//
// Mainwindow.h
//
// Copyright 2013, 2014 by John Pietrzak
//
// This file contains the main window declaration for Orecchiette.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QString>
#include <QTimer>
#include <QTime>
#include "oreencoding.h"

class OrePreferencesForm;
class OreVideoMonitorForm;
class OreDocumentationForm;
class OreAboutForm;
class OreGst;
class OreDBus;

namespace Ui {
  class MainWindow;
}

enum OreStatus
{
  Idle_Status,
  RecordingInput_Status,
  RecordingOutput_Status,
  RecordingBoth_Status,
  RecordingVideo_Status,
  Recording_Status,
  Playing_Status,
  Paused_Status
};

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  enum ScreenOrientation {
    ScreenOrientationLockPortrait,
    ScreenOrientationLockLandscape,
    ScreenOrientationAuto
  };

  explicit MainWindow(QWidget *parent = 0);
  virtual ~MainWindow();

  // Note that this will only have an effect on Symbian and Fremantle.
  void setOrientation(ScreenOrientation orientation);

  void showExpanded();

  void updateStatus(
    OreStatus status);

  void startNewStatus(
    OreStatus status);

  void pauseDisplay();

  void continueDisplay();

  bool recordingVideo();

public slots:
  // I'd like to use these slots from a sub-window:
  void on_pauseButton_clicked();
  void on_stopButton_clicked();

private slots:
  void on_actionPreferences_triggered();
  void on_actionDocumentation_triggered();
  void on_actionAbout_triggered();

/*
  void on_inputButton_clicked();
  void on_outputButton_clicked();
  void on_bothButton_clicked();
  void on_screenButton_clicked();
*/

  void on_recordButton_clicked();
  void on_playButton_clicked();

  void startRecordingCall();
  void stopRecordingCall();

  void updateStatusTime();

  void on_audioComboBox_currentIndexChanged(int index);
  void on_videoComboBox_currentIndexChanged(int index);

private:
  OrePreferencesForm *preferencesForm;
  OreVideoMonitorForm *videoMonitorForm;
  OreDocumentationForm *documentationForm;
  OreAboutForm *aboutForm;

  OreGst *myGst;
  OreDBus *myDBus;

/*
  bool recordInput;
  bool recordOutput;
  bool recordVideo;
*/
  OreStatus lastActiveStatus;

  OreAudioSource audioChoice;
  OreVideoSource videoChoice;

  QTime runningTime;
  int elapsedTime;
  QTimer secondTimer;

  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
