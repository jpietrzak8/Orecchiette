//
// mainwindow.cpp
//
// Copyright 2013, 2014 by John Pietrzak (jpietrzak8@gmail.com)
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "orepreferencesform.h"
#include "orevideomonitorform.h"
#include "oredocumentationform.h"
#include "oreaboutform.h"
#include "oregst.h"
#include "oredbus.h"
//#include "oremjpegdialog.h"
#include "oreexception.h"

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QString>
#include <QFileDialog>
#include <QSettings>


MainWindow::MainWindow(
  QWidget *parent)
  : QMainWindow(parent),
    preferencesForm(0),
    videoMonitorForm(0),
    documentationForm(0),
    aboutForm(0),
    myGst(0),
    myDBus(0),
//    myUrlDialog(0),
    lastActiveStatus(Playing_Status), // this is a hack
    audioChoice(Microphone_Audio),
    videoChoice(No_Video),
    elapsedTime(0),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  ui->pauseButton->setEnabled(false);
  ui->stopButton->setEnabled(false);

  updateStatus(Idle_Status);

  setAttribute(Qt::WA_Maemo5StackedWindow);

  preferencesForm = new OrePreferencesForm(this);
  videoMonitorForm = new OreVideoMonitorForm(this);

  myGst = new OreGst(this, videoMonitorForm->getWindowId());
  myDBus = new OreDBus();
//  myUrlDialog = new OreMJpegDialog(this);

  connect(
    myDBus, SIGNAL(callStarted()),
    this, SLOT(startRecordingCall()));

  connect(
    myDBus, SIGNAL(callTerminated()),
    this, SLOT(stopRecordingCall()));

  connect(
    myDBus, SIGNAL(playingAllowed()),
    this, SLOT(startPlaying()));

  connect(
    myDBus, SIGNAL(playingDenied()),
    this, SLOT(stopPlaying()));

  connect(
    preferencesForm, SIGNAL(encodingChanged(AudioEncoding)),
    myGst, SLOT(setAudioEncoding(AudioEncoding)));

  secondTimer.setInterval(1000);

  connect(
    &secondTimer, SIGNAL(timeout()),
    this, SLOT(updateStatusTime()));

  // Set up the combo boxes:
  QSettings settings("pietrzak.org", "Orecchiette");
  if (settings.contains("AudioSourceChoice"))
  {
    QString asc = settings.value("AudioSourceChoice").toString();

    if (asc == "Microphone_Audio")
    {
      ui->audioComboBox->setCurrentIndex(1);
    }
    else if (asc == "Speaker_Audio")
    {
      ui->audioComboBox->setCurrentIndex(2);
    }
    else if (asc == "MicrophoneAndSpeaker_Audio")
    {
      ui->audioComboBox->setCurrentIndex(3);
    }
    else
    {
      ui->audioComboBox->setCurrentIndex(0);
    }
  }

  if (settings.contains("VideoSourceChoice"))
  {
    QString vsc = settings.value("VideoSourceChoice").toString();

    if (vsc == "Screen_Video")
    {
      ui->videoComboBox->setCurrentIndex(1);
    }
    else if (vsc == "BackCamera_Video")
    {
      ui->videoComboBox->setCurrentIndex(2);
    }
    else if (vsc == "FrontCamera_Video")
    {
      ui->videoComboBox->setCurrentIndex(3);
    }
/*
    else if (vsc == "MJpegStream_Video")
    {
      ui->videoComboBox->setCurrentIndex(4);
    }
*/
    else
    {
      ui->videoComboBox->setCurrentIndex(0);
    }
  }

  if (preferencesForm)
  {
/*
    switch (preferencesForm->source)
    {
    case OrePreferencesForm::Microphone:
      ui->inputButton->click();
      break;
    case OrePreferencesForm::Speaker:
      ui->outputButton->click();
      break;
    case OrePreferencesForm::Both:
    default:
      ui->bothButton->click();
      break;
    }
*/

    if (preferencesForm->recordOnStartUp())
    {
      // Act like the Record button's been pressed immediately after initialization
      ui->recordButton->click();
    }
  }
}


MainWindow::~MainWindow()
{
  QSettings settings("pietrzak.org", "Orecchiette");

  switch (audioChoice)
  {
  case Microphone_Audio:
    settings.setValue("AudioSourceChoice", "Microphone_Audio");
    break;
  case Speaker_Audio:
    settings.setValue("AudioSourceChoice", "Speaker_Audio");
    break;
  case MicrophoneAndSpeaker_Audio:
    settings.setValue("AudioSourceChoice", "MicrophoneAndSpeaker_Audio");
    break;
  case No_Audio:
  default:
    settings.setValue("AudioSourceChoice", "No_Audio");
    break;
  };

  switch (videoChoice)
  {
  case Screen_Video:
    settings.setValue("VideoSourceChoice", "Screen_Video");
    break;
  case BackCamera_Video:
    settings.setValue("VideoSourceChoice", "BackCamera_Video");
    break;
  case FrontCamera_Video:
    settings.setValue("VideoSourceChoice", "FrontCamera_Video");
    break;
/*
  case MJpegStream_Video:
    settings.setValue("VideoSourceChoice", "MJpegStream_Video");
    break;
*/
  case No_Video:
  default:
    settings.setValue("VideoSourceChoice", "No_Video");
    break;
  };

  if (videoMonitorForm) delete videoMonitorForm;
  if (preferencesForm) delete preferencesForm;
  if (documentationForm) delete documentationForm;
  if (aboutForm) delete aboutForm;
  if (myGst) delete myGst;
  if (myDBus) delete myDBus;
//  if (myUrlDialog) delete myUrlDialog;

  delete ui;
}


void MainWindow::setOrientation(ScreenOrientation orientation)
{
#if defined(Q_OS_SYMBIAN)
    // If the version of Qt on the device is < 4.7.2, that attribute won't work
    if (orientation != ScreenOrientationAuto) {
        const QStringList v = QString::fromAscii(qVersion()).split(QLatin1Char('.'));
        if (v.count() == 3 && (v.at(0).toInt() << 16 | v.at(1).toInt() << 8 | v.at(2).toInt()) < 0x040702) {
            qWarning("Screen orientation locking only supported with Qt 4.7.2 and above");
            return;
        }
    }
#endif // Q_OS_SYMBIAN

    Qt::WidgetAttribute attribute;
    switch (orientation) {
#if QT_VERSION < 0x040702
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
    case ScreenOrientationLockPortrait:
        attribute = static_cast<Qt::WidgetAttribute>(128);
        break;
    case ScreenOrientationLockLandscape:
        attribute = static_cast<Qt::WidgetAttribute>(129);
        break;
    default:
    case ScreenOrientationAuto:
        attribute = static_cast<Qt::WidgetAttribute>(130);
        break;
#else // QT_VERSION < 0x040702
    case ScreenOrientationLockPortrait:
        attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
        attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
        attribute = Qt::WA_AutoOrientation;
        break;
#endif // QT_VERSION < 0x040702
    };
    setAttribute(attribute, true);
}


void MainWindow::showExpanded()
{
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
    showFullScreen();
#elif defined(Q_WS_MAEMO_5)
    showMaximized();
#else
    show();
#endif
}


bool MainWindow::recordingVideo()
{
//  return recordVideo;
  return videoChoice != No_Video;
}


void MainWindow::on_actionPreferences_triggered()
{
  preferencesForm->show();
}


void MainWindow::on_actionDocumentation_triggered()
{
  if (!documentationForm)
  {
    documentationForm = new OreDocumentationForm(this);
  }

  documentationForm->show();
}


void MainWindow::on_actionAbout_triggered()
{
  if (!aboutForm)
  {
    aboutForm = new OreAboutForm(this);
  }

  aboutForm->show();
}


/*
void MainWindow::on_inputButton_clicked()
{
  recordInput = true;
  recordOutput = false;
  recordVideo = false;
  preferencesForm->source = OrePreferencesForm::Microphone;
}


void MainWindow::on_outputButton_clicked()
{
  recordOutput = true;
  recordInput = false;
  recordVideo = false;
  preferencesForm->source = OrePreferencesForm::Speaker;
}


void MainWindow::on_bothButton_clicked()
{
  recordInput = true;
  recordOutput = true;
  recordVideo = false;
  preferencesForm->source = OrePreferencesForm::Both;
}


void MainWindow::on_screenButton_clicked()
{
  recordInput = true;
  recordOutput = true;
  recordVideo = true;
  preferencesForm->source = OrePreferencesForm::Both;
}
*/


void MainWindow::on_recordButton_clicked()
{
  if (myGst->gstreamerInUse())
  {
    try
    {
      myGst->stopCurrentElement();

      ui->recordButton->setIcon(QIcon(":/icons/notrecording.png"));

      startNewStatus(Idle_Status);
    }
    catch(OreException &e)
    {
      e.display();
    }
  }
  else
  {
    try
    {
/*
      if (videoChoice == MJpegStream_Video)
      {
        // Ask for the stream URL:
        if (myUrlDialog->exec())
        {
          // Set the url:
          myGst->setMJpegStreamUrl(myUrlDialog->getUrl());
        }
      }
*/

      myGst->startRecording(
        *preferencesForm,
        myDBus->btDeviceInUse(),
        preferencesForm->getNextFilename(),
        audioChoice,
        videoChoice);

      startNewStatus(Recording_Status);

/*
      if (recordVideo)
      {
        // For now, recordVideo implies both Input and Output audio as well.
        myGst->startRecordingScreen(
          myDBus->btDeviceInUse(),
          preferencesForm->getNextFilename());

        startNewStatus(RecordingVideo_Status);
      }
      else if (recordInput)
      {
        if (recordOutput)
        {
          myGst->startRecordingCall(
            myDBus->btDeviceInUse(),
            preferencesForm->getNextFilename());

          startNewStatus(RecordingBoth_Status);
        }
        else
        {
          myGst->startRecordingMicrophone(
            myDBus->btDeviceInUse(),
            preferencesForm->getNextFilename());

          startNewStatus(RecordingInput_Status);
        }
      }
      else
      {
        myGst->startRecordingSpeaker(
          myDBus->btDeviceInUse(),
          preferencesForm->getNextFilename());

        startNewStatus(RecordingOutput_Status);
      }
*/

      ui->recordButton->setIcon(QIcon(":/icons/recording.png"));

      // Force landscape mode here.
      // Turn auto off:
      setAttribute(static_cast<Qt::WidgetAttribute>(130), false);
      // Turn landscape on:
      setAttribute(static_cast<Qt::WidgetAttribute>(129), true);

      if (videoChoice == BackCamera_Video)
      {
        videoMonitorForm->showBackCamera();
      }
      else if (videoChoice == FrontCamera_Video)
      {
        videoMonitorForm->showFrontCamera();
      }
/*
      else if (videoChoice == MJpegStream_Video)
      {
        videoMonitorForm->showVideo();
      }
*/
    }
    catch (OreException &e)
    {
      e.display();
    }
  }

  ui->pauseButton->setEnabled(true);
  ui->stopButton->setEnabled(true);
}


void MainWindow::on_playButton_clicked()
{
  // Don't do anything if we're busy:
  if (myGst->gstreamerInUse()) return;

  QString filename = QFileDialog::getOpenFileName(
    this,
    "Choose Audio File",
    preferencesForm->getAudioDirectory());

  // Just return if the user didn't select a file:
  if (filename.isEmpty()) return;

  filenameToPlay = filename;

  myDBus->requestToPlay();

  ui->pauseButton->setEnabled(true);
  ui->stopButton->setEnabled(true);
}


void MainWindow::on_pauseButton_clicked()
{
  // Only pause if we are currently doing something:
  if (!myGst->gstreamerInUse()) return;

  try
  {
    myGst->pauseOrContinue();
  }
  catch(OreException &e)
  {
    e.display();
  }
}


void MainWindow::on_stopButton_clicked()
{
  // Don't do anything if we're not playing or recording:
  if (!myGst->gstreamerInUse()) return;

  try
  {
    myGst->stopCurrentElement();

    ui->recordButton->setIcon(QIcon(":/icons/notrecording.png"));

    startNewStatus(Idle_Status);
  }
  catch(OreException &e)
  {
    e.display();
  }

  ui->pauseButton->setEnabled(false);
  ui->stopButton->setEnabled(false);
}


void MainWindow::startRecordingCall()
{
  // Only record phone calls if authorized to do so:
  if (!preferencesForm->recordPhoneCalls()) return;

  if (myGst->gstreamerInUse())
  {
    try
    {
      myGst->stopCurrentElement();
    }
    catch(...)
    {
      // In this case, we'll silently ignore any errors.
    }
  }

  try
  {
    myGst->startRecordingCall(
      *preferencesForm,
      myDBus->btDeviceInUse(),
      preferencesForm->getNextFilename());

    ui->recordButton->setIcon(QIcon(":/icons/recording.png"));

    startNewStatus(RecordingBoth_Status);
  }
  catch(OreException &e)
  {
    e.display();
  }
}


void MainWindow::stopRecordingCall()
{
  // If we are recording a call, consider this the equivalent of hitting
  // the "stop" button.
  if (myGst->currentlyRecordingCall())
  {
    on_stopButton_clicked();
  }
}


void MainWindow::updateStatus(
  OreStatus status)
{
  QString statusString;
  switch(status)
  {
  case RecordingInput_Status:
    statusString = "Status: Recording Input ";
    break;

  case RecordingOutput_Status:
    statusString = "Status: Recording Output ";
    break;

  case RecordingBoth_Status:
    statusString = "Status: Recording Both ";
    break;

  case RecordingVideo_Status:
    statusString = "Status: Recording Video ";
    break;

  case Recording_Status:
    statusString = "Status: Recording ";
    switch (audioChoice)
    {
    case Microphone_Audio:
      statusString += "Mic ";
      break;

    case Speaker_Audio:
      statusString += "Speaker ";
      break;

    case MicrophoneAndSpeaker_Audio:
      statusString += "Mic and Speaker ";
      break;

    default:
      break;
    };

    if ((audioChoice != No_Audio) && (videoChoice != No_Video))
    {
      statusString += "and ";
    }

    switch (videoChoice)
    {
    case Screen_Video:
      statusString += "screen ";
      break;

    case BackCamera_Video:
      statusString += "rear camera ";
      break;

    case FrontCamera_Video:
      statusString += "front camera ";
      break;

    default:
      break;
    };

    break;

  case Playing_Status:
    statusString = "Status: Playing ";
    break;

  case Paused_Status:
    statusString = "Status: Paused ";
    break;

  case Idle_Status:
  default:
    statusString = "Status: Idle";
    break;
  }

  if (status != Idle_Status)
  {
    QTime elapsedTimeObj(0,0);
    statusString += elapsedTimeObj.addMSecs(elapsedTime).toString(Qt::TextDate);

    if (status != Paused_Status)
    {
      lastActiveStatus = status;
    }
  }

  ui->statusLabel->setText(statusString);
}


void MainWindow::startNewStatus(
  OreStatus status)
{
  if (status == Idle_Status)
  {
    elapsedTime += runningTime.elapsed();
    secondTimer.stop();
  }
  else
  {
    elapsedTime = 0;
    runningTime.start();
    secondTimer.start();
  }

  updateStatus(status);
}


void MainWindow::pauseDisplay()
{
  elapsedTime += runningTime.elapsed();

  secondTimer.stop();

  updateStatus(Paused_Status);
}


void MainWindow::continueDisplay()
{
  runningTime.start();

  updateStatus(lastActiveStatus);

  secondTimer.start();
}


void MainWindow::updateStatusTime()
{
  elapsedTime += runningTime.restart();

  updateStatus(lastActiveStatus);
}

void MainWindow::on_audioComboBox_currentIndexChanged(int index)
{
  // Need to coordinate this with the gui!

  switch (index)
  {
  case 0:
    audioChoice = No_Audio;
    break;

  case 1:
  default:
    audioChoice = Microphone_Audio;
    break;

  case 2:
    audioChoice = Speaker_Audio;
    break;

  case 3:
    audioChoice = MicrophoneAndSpeaker_Audio;
  }
}

void MainWindow::on_videoComboBox_currentIndexChanged(int index)
{
  // Need to coordinate this with the gui!
  switch (index)
  {
  case 0:
  default:
    videoChoice = No_Video;
    break;

  case 1:
    videoChoice = Screen_Video;
    break;

  case 2:
    videoChoice = BackCamera_Video;
    break;

  case 3:
    videoChoice = FrontCamera_Video;
    break;

/*
  case 4:
    videoChoice = MJpegStream_Video;
    break;
*/
  }
}


void MainWindow::startPlaying()
{
  try
  {
    // Very crude method of determining whether video is present: just
    // checking for a ".mkv" suffix.
    if (filenameToPlay.endsWith(".mkv"))
    {
      // Force landscape mode here.
      // Turn auto off:
      setAttribute(static_cast<Qt::WidgetAttribute>(130), false);
      // Turn landscape on:
      setAttribute(static_cast<Qt::WidgetAttribute>(129), true);

      myGst->startPlaying(true, filenameToPlay);

      videoMonitorForm->showVideo();
    }
    else
    {
      myGst->startPlaying(false, filenameToPlay);
    }
    startNewStatus(Playing_Status);
  }
  catch (OreException &e)
  {
    e.display();
  }
}


void MainWindow::stopPlaying()
{
  // For now, this will be identical to hitting the "stop" button, although
  // it might be more akin to "pause"...
  on_stopButton_clicked();
}
