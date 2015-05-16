//
// orevideomonitorform.h
//
// Copyright 2015 by John Pietrzak  (jpietrzak8@gmail.com)
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

#ifndef OREVIDEOMONITORFORM_H
#define OREVIDEOMONITORFORM_H

#include <QWidget>

#include "orecameracontrol.h"

class MainWindow;
class QCloseEvent;

namespace Ui {
class OreVideoMonitorForm;
}

class OreVideoMonitorForm : public QWidget
{
  Q_OBJECT
  
public:
  OreVideoMonitorForm(
    MainWindow *parent);

  ~OreVideoMonitorForm();

  unsigned int getWindowId();

  void showBackCamera();
  void showFrontCamera();
  void showVideo();

protected:
  void closeEvent(
    QCloseEvent *event);
  
private slots:
  void on_lightButton_clicked();

  void on_focusUpButton_pressed();
  void on_focusUpButton_released();

  void on_focusDownButton_pressed();
  void on_focusDownButton_released();

  void on_expUpButton_pressed();
  void on_expUpButton_released();

  void on_expDownButton_pressed();
  void on_expDownButton_released();

  void on_stopButton_clicked();

  void on_pauseButton_clicked();

private:
  Ui::OreVideoMonitorForm *ui;

  MainWindow *mainWindow;

  bool showingBackCamera;

  OreCameraControl camera;
};

#endif // OREVIDEOMONITORFORM_H
