//
// Mainwindow.h
//
// Copyright 2013 by John Pietrzak
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

class OrePreferencesForm;
class OreDocumentationForm;
class OreAboutForm;
class OreGst;
class OreDBus;

namespace Ui {
  class MainWindow;
}

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
    QString status);

  void pauseStatus();

  void continueStatus();

private slots:
  void on_actionPreferences_triggered();
  void on_actionDocumentation_triggered();
  void on_actionAbout_triggered();

  void on_recordButton_clicked();
  void on_inputButton_toggled(
    bool checked);

  void on_playButton_clicked();
  void on_pauseButton_clicked();
  void on_stopButton_clicked();

  void startRecordingCall();
  void stopRecordingCall();

private:
  OrePreferencesForm *preferencesForm;
  OreDocumentationForm *documentationForm;
  OreAboutForm *aboutForm;

  OreGst *myGst;
  OreDBus *myDBus;

  bool recordFromMic;
  QString statusBuffer;

  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
