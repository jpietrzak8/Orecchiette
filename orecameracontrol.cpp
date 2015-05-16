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

#include "orecameracontrol.h"
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

// 5/16/15: Having trouble setting exposure levels on the front camera, so
// for now I'm going to just deal with the back camera instead.

OreCameraControl::OreCameraControl()
  : backCameraFD(-1),
    frontCameraFD(-1),
    originalFocusLevel(0),
    focusLevel(0),
    backExposureLevel(33000),
    frontExposureLevel(33000),
    timer(0),
    torchOn(false)
{
  // First, need to ensure that the camera is not running:
  QProcess process;
  process.execute("/usr/sbin/dsmetool -k /usr/bin/camera-ui");

/*
  backCameraFD = open("/dev/video0", O_RDWR, 0);

  if (backCameraFD == -1)
  {
    QString err("Failed to connect to /dev/video0.\nError is: ");
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }
  else
  {
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;

    if (ioctl(backCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
    {
      qWarning() << "Unable to query focus, err: " << strerror(errno);
    }
    else
    {
      originalFocusLevel = ctrl.value;
      focusLevel = ctrl.value;
      qWarning() << "Focus value: " << ctrl.value;
    }

    // Gently initialize the focus level:
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(initializeFocus()));
    timer->start(50);
  }

  close(backCameraFD);
  backCameraFD = -1;
*/
}


OreCameraControl::~OreCameraControl()
{
  if (torchOn) toggleTorch();

/*
  // Reset the focus:
  timespec sleeptime;
  sleeptime.tv_sec = 0;
  sleeptime.tv_nsec = 50000000;
  timespec remainingtime;
  switchToBackCamera();
  if (focusLevel > originalFocusLevel)
  {
    while (focusLevel > originalFocusLevel)
    {
      focusDown();
      if (nanosleep(&sleeptime, &remainingtime) == -1)
      {
        qWarning() << "Failed to sleep, error: " << strerror(errno);
      }
    }
  }
  else
  {
    while (focusLevel < originalFocusLevel)
    {
      focusUp();
      if (nanosleep(&sleeptime, &remainingtime) == -1)
      {
        qWarning() << "Failed to sleep, error: " << strerror(errno);
      }
    }
  }
*/

  if (backCameraFD >= 0)
  {
    close(backCameraFD);
    backCameraFD = -1;
  }

  if (frontCameraFD >= 0)
  {
    close(frontCameraFD);
    frontCameraFD = -1;
  }

  if (timer) delete timer;

  // Restart the camera:
  QProcess process;
  process.execute("/usr/sbin/dsmetool -t /usr/bin/camera-ui");
}


void OreCameraControl::turnOffTorch()
{
  if (torchOn) toggleTorch();
}


void OreCameraControl::toggleTorch()
{
  struct v4l2_control ctrl;

  // Sanity check:
  if (backCameraFD == -1)
  {
    // An error message should have already been shown...
    return;
  }

  ctrl.id = V4L2_CID_TORCH_INTENSITY;

  if (torchOn)
  {
    ctrl.value = 0;
    torchOn = false;
  }
  else
  {
    ctrl.value = 1;
    torchOn = true;
  }

  if (ioctl(backCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    QString err("Failed to set torch intensity to ");
    err += ctrl.value;
    err += "\nError is ";
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }

/*
  ctrl.id = V4L2_CID_EXPOSURE;
  if (torchOn)
  {
    ctrl.value = ledLightExposureLevel;
  }
  else
  {
    ctrl.value = sunlightExposureLevel;
  }

  if (ioctl(backCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to set exposure to " << ctrl.value;
    qWarning() << ", error is: " << strerror(errno);
  }
*/
}


void OreCameraControl::startIncrementingFocus()
{
  switchToBackCamera();

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;

  if (ioctl(backCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
  {
    // Don't try to move the lens if you don't know the current focus level!
    qWarning() << "Unable to query focus, err: " << strerror(errno);
    return;
  }
  else
  {
    focusLevel = ctrl.value;
  }

  if (timer) delete timer;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(focusUp()));
  timer->start(100);
}


void OreCameraControl::startDecrementingFocus()
{
  switchToBackCamera();

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;

  if (ioctl(backCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
  {
    // Don't try to move the lens if you don't know the current focus level!
    qWarning() << "Unable to query focus, err: " << strerror(errno);
    return;  // Should return error code here
  }
  else
  {
    focusLevel = ctrl.value;
  }

  if (timer) delete timer;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(focusDown()));
  timer->start(100);
}


void OreCameraControl::startIncrementingExposure()
{
  switchToBackCamera();

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  if (ioctl(backCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to query exposure, err: " << strerror(errno);
    return;
  }
  else
  {
    backExposureLevel = ctrl.value;
qDebug() << "exposure: " << ctrl.value;
  }

  if (timer) delete timer;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(exposureUp()));
  timer->start(100);
}


void OreCameraControl::startDecrementingExposure()
{
  switchToBackCamera();

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  if (ioctl(backCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to query exposure, err: " << strerror(errno);
    return;
  }
  else
  {
    backExposureLevel = ctrl.value;
qDebug() << "exposure: " << ctrl.value;
  }

  if (timer) delete timer;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(exposureDown()));
  timer->start(100);
}


void OreCameraControl::startIncrementingFrontExposure()
{
/* 5/16/15: disabling front camera controls for now.
  switchToFrontCamera();

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  if (ioctl(frontCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to query exposure, err: " << strerror(errno);
    return;
  }
  else
  {
    frontExposureLevel = ctrl.value;
qDebug() << "exposure: " << ctrl.value;
  }

  if (timer) delete timer;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(frontExposureUp()));
  timer->start(100);
*/
}


void OreCameraControl::startDecrementingFrontExposure()
{
/* 5/16/15: disabling front camera controls for now.
  switchToFrontCamera();

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  if (ioctl(frontCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to query exposure, err: " << strerror(errno);
    return;
  }
  else
  {
    frontExposureLevel = ctrl.value;
qDebug() << "exposure: " << ctrl.value;
  }

  if (timer) delete timer;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(frontExposureDown()));
  timer->start(100);
*/
}


void OreCameraControl::stopTimer()
{
  if (timer)
  {
    delete timer;
    timer = 0;
  }
}


void OreCameraControl::initializeFocus()
{
  // sanity check:
  if (backCameraFD == -1)
  {
    return;
  }

  // We'll reach the end when we've reached 700:
  if (focusLevel >= 700)
  {
    // Stop the timer and return:
    // (Could probably do this in a cleaner way...)
    if (timer)
    {
      delete timer;
      timer = 0;
    }
    return;
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;

  focusLevel += 10;
  ctrl.value = focusLevel;

  if (ioctl(backCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to set focus to: " << ctrl.value
      << ", err: " << strerror(errno);
    return;
  }
}


void OreCameraControl::focusUp()
{
  // sanity check:
  if (backCameraFD == -1)
  {
    return;
  }

  // Don't bother if we've reached the end:
  if (focusLevel == 1023)
  {
    return;
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;

  focusLevel += 10;
  if (focusLevel > 1023)
  {
    focusLevel = 1023;
  }

  ctrl.value = focusLevel;

  if (ioctl(backCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    QString err("Unable to set focus to ");
    err += ctrl.value;
    err += "\nError is ";
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }
//qDebug() << "focus level: " << ctrl.value;
}


void OreCameraControl::focusDown()
{
  // sanity check:
  if (backCameraFD == -1)
  {
    return;
  }

  // Don't bother if we've reached the end:
  if (focusLevel == 0)
  {
    return;
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;

  focusLevel -= 10;
  if (focusLevel < 0)
  {
    focusLevel = 0;
  }

  ctrl.value = focusLevel;

  if (ioctl(backCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    QString err("Unable to set focus to ");
    err += ctrl.value;
    err += "\nError is ";
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }
//qDebug() << "Focus level: " << ctrl.value;
}


void OreCameraControl::exposureUp()
{
  // sanity check:
  if (backCameraFD == -1)
  {
    return;
  }

  int exposureLevel = backExposureLevel;

  // Stop if we've reached the end.
  // Not sure about the units here...
  if (exposureLevel == 65535) 
  {
    return;
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  exposureLevel += 500;
  if (exposureLevel > 65535)
  {
    exposureLevel = 65535;
  }

  ctrl.value = exposureLevel;

//qDebug() << "Setting exposure to " << ctrl.value;
  if (ioctl(backCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    QString err("Unable to set exposure to ");
    err += ctrl.value;
    err += "\nError is ";
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }
  else
  {
    // Store the new level:
    backExposureLevel = exposureLevel;
  }
}


void OreCameraControl::exposureDown()
{
  // sanity check:
  if (backCameraFD == -1)
  {
    return;
  }

  int exposureLevel = backExposureLevel;

  // Stop if we've reached the end.
  if (exposureLevel == 1) 
  {
    return;
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  exposureLevel -= 500;
  if (exposureLevel < 1)
  {
    exposureLevel = 1;
  }

  ctrl.value = exposureLevel;

//qDebug() << "Setting exposure to " << ctrl.value;
  if (ioctl(backCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    QString err("Unable to set exposure to ");
    err += ctrl.value;
    err += "\nError is ";
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }
  else
  {
    // Store the new level:
    backExposureLevel = exposureLevel;
  }
}


void OreCameraControl::frontExposureUp()
{
  // sanity check:
  if (frontCameraFD == -1)
  {
    return;
  }

  int exposureLevel = frontExposureLevel;

  // Stop if we've reached the end.
  // Not sure about the units here...
  if (exposureLevel == 65535) 
  {
    return;
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  exposureLevel += 500;
  if (exposureLevel > 65535)
  {
    exposureLevel = 65535;
  }

  ctrl.value = exposureLevel;

//qDebug() << "Setting exposure to " << ctrl.value;
  if (ioctl(frontCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    QString err("Unable to set exposure to ");
    err += ctrl.value;
    err += "\nError is ";
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }
  else
  {
    // Store the new level:
    frontExposureLevel = exposureLevel;
qDebug() << "new exposure: " << ctrl.value;
  }
}


void OreCameraControl::frontExposureDown()
{
  // sanity check:
  if (frontCameraFD == -1)
  {
    return;
  }

  int exposureLevel = frontExposureLevel;

  // Stop if we've reached the end.
  if (exposureLevel == 1) 
  {
    return;
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE;

  exposureLevel -= 500;
  if (exposureLevel < 1)
  {
    exposureLevel = 1;
  }

  ctrl.value = exposureLevel;

//qDebug() << "Setting exposure to " << ctrl.value;
  if (ioctl(frontCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    QString err("Unable to set exposure to ");
    err += ctrl.value;
    err += "\nError is ";
    err += strerror(errno);
//    QMaemo5InformationBox::information(this, err);
    qWarning() << err;
  }
  else
  {
    // Store the new level:
    frontExposureLevel = exposureLevel;
  }
}


void OreCameraControl::switchToFrontCamera()
{
/* 5/16/15: Disabling front camera controls for now.
  // Sanity check:
  if (frontCameraFD != -1)
  {
    // Front camera already active
    return;
  }

  if (backCameraFD >= 0)
  {
    close(backCameraFD);
    backCameraFD = -1;
  }

  frontCameraFD = open("/dev/video1", O_RDWR, 0);

  if (frontCameraFD == -1)
  {
    QString err("Failed to connect to /dev/video1.\nError is: ");
    err += strerror(errno);
    qWarning() << err;
  }

  // Force manual exposure:
  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_EXPOSURE_AUTO;

  if (ioctl(frontCameraFD, VIDIOC_G_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to retrieve exp settings: " << strerror(errno);
  }
  else
  {
    qWarning() << "Exposure setting is: " << ctrl.value;
  }

  ctrl.value = V4L2_EXPOSURE_MANUAL;
  if (ioctl(frontCameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
  {
    qWarning() << "Unable to set exposure to manual: " << strerror(errno);
  }
*/
}


void OreCameraControl::switchToBackCamera()
{
  // Sanity check:
  if (backCameraFD != -1)
  {
    // Back camera already active
    return;
  }

  if (frontCameraFD >= 0)
  {
    close(frontCameraFD);
    frontCameraFD = -1;
  }

  backCameraFD = open("/dev/video0", O_RDWR, 0);

  if (backCameraFD == -1)
  {
    QString err("Failed to connect to /dev/video0.\nError is: ");
    err += strerror(errno);
    qWarning() << err;
  }
}
