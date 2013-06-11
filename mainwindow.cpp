//
// mainwindow.cpp
//
// Copyright 2013 by John Pietrzak (jpietrzak8@gmail.com)
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
#include "oredocumentationform.h"
#include "oreaboutform.h"
#include "oregst.h"
#include "oredbus.h"
#include "oreexception.h"

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QString>
#include <QFileDialog>


MainWindow::MainWindow(
  QWidget *parent)
  : QMainWindow(parent),
    preferencesForm(0),
    documentationForm(0),
    aboutForm(0),
    myGst(0),
    myDBus(0),
    recordInput(true),
    recordOutput(false),
    statusBuffer("Status: Idle"),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  updateStatus("Status: Idle");

  setAttribute(Qt::WA_Maemo5StackedWindow);

  preferencesForm = new OrePreferencesForm(this);

  myGst = new OreGst(this);
  myDBus = new OreDBus();

  connect(
    myDBus, SIGNAL(callStarted()),
    this, SLOT(startRecordingCall()));

  connect(
    myDBus, SIGNAL(callTerminated()),
    this, SLOT(stopRecordingCall()));

  connect(
    preferencesForm, SIGNAL(encodingChanged(AudioEncoding)),
    myGst, SLOT(setAudioEncoding(AudioEncoding)));
}


MainWindow::~MainWindow()
{
  if (preferencesForm) delete preferencesForm;
  if (documentationForm) delete documentationForm;
  if (aboutForm) delete aboutForm;
  if (myGst) delete myGst;
  if (myDBus) delete myDBus;

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


void MainWindow::on_inputButton_clicked()
{
  recordInput = true;
  recordOutput = false;
}


void MainWindow::on_outputButton_clicked()
{
  recordOutput = true;
  recordInput = false;
}


void MainWindow::on_bothButton_clicked()
{
  recordInput = true;
  recordOutput = true;
}


void MainWindow::on_recordButton_clicked()
{
  if (myGst->gstreamerInUse())
  {
    try
    {
      myGst->stopCurrentElement();

      ui->recordButton->setIcon(QIcon(":/icons/notrecording.png"));

      updateStatus("Status: Idle");
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
      if (recordInput)
      {
        if (recordOutput)
        {
          myGst->startRecordingCall(
            myDBus->btDeviceInUse(),
            preferencesForm->getNextFilename());

          updateStatus("Status: Recording Both Streams");
        }
        else
        {
          myGst->startRecordingMicrophone(
            myDBus->btDeviceInUse(),
            preferencesForm->getNextFilename());

          updateStatus("Status: Recording Input Stream");
        }
      }
      else
      {
        myGst->startRecordingSpeaker(
          myDBus->btDeviceInUse(),
          preferencesForm->getNextFilename());

        updateStatus("Status: Recording Output Stream");
      }

      ui->recordButton->setIcon(QIcon(":/icons/recording.png"));
    }
    catch (OreException &e)
    {
      e.display();
    }
  }
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

  try
  {
    myGst->startPlaying(filename);
    updateStatus("Status: Playing Audio Stream");
  }
  catch (OreException &e)
  {
    e.display();
  }
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

    updateStatus("Status: Idle");
  }
  catch(OreException &e)
  {
    e.display();
  }
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
      myDBus->btDeviceInUse(),
      preferencesForm->getNextFilename());

    ui->recordButton->setIcon(QIcon(":/icons/recording.png"));

    updateStatus("Status: Recording Phone Call");
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
  QString status)
{
  ui->statusLabel->setText(status);
}


void MainWindow::pauseStatus()
{
  statusBuffer = ui->statusLabel->text();

  ui->statusLabel->setText("Status: Paused");
}


void MainWindow::continueStatus()
{
  ui->statusLabel->setText(statusBuffer);
}
