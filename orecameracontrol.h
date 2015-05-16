//
// orecameracontrol.cpp
//
// Copyright 2015 by John Pietrzak (jpietrzak8@gmail.com)
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

#ifndef ORECAMERACONTROL_H
#define ORECAMERACONTROL_H

#include <QObject>

class QTimer;

class OreCameraControl: public QObject
{
  Q_OBJECT

public:
  OreCameraControl();

  ~OreCameraControl();

  void toggleTorch();
  void turnOffTorch();

  void startIncrementingFocus();
  void startDecrementingFocus();

  void startIncrementingExposure();
  void startDecrementingExposure();

  void startIncrementingFrontExposure();
  void startDecrementingFrontExposure();

  void stopTimer();

private slots:
  void initializeFocus();
  void focusUp();
  void focusDown();
  void exposureUp();
  void exposureDown();
  void frontExposureUp();
  void frontExposureDown();

private:
  void switchToBackCamera();
  void switchToFrontCamera();

  int backCameraFD;
  int frontCameraFD;

  int originalFocusLevel;
  int focusLevel;

  int backExposureLevel;
  int frontExposureLevel;

  QTimer *timer;

  bool torchOn;
};

#endif // ORECAMERACONTROL_H
