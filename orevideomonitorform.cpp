//
// orevideomonitorform.cpp
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

#include "orevideomonitorform.h"
#include "ui_orevideomonitorform.h"

#include "mainwindow.h"

OreVideoMonitorForm::OreVideoMonitorForm(
  MainWindow *parent)
  : QWidget(parent),
    ui(new Ui::OreVideoMonitorForm),
    mainWindow(parent),
    showingBackCamera(false)
{
  ui->setupUi(this);

  setAttribute(Qt::WA_Maemo5StackedWindow);
  setWindowFlags(windowFlags() | Qt::Window);
}

OreVideoMonitorForm::~OreVideoMonitorForm()
{
  delete ui;
}

unsigned int OreVideoMonitorForm::getWindowId()
{
  return ui->videoWidget->winId();
}


void OreVideoMonitorForm::showBackCamera()
{
  ui->lightButton->setEnabled(true);
  ui->focusUpButton->setEnabled(true);
  ui->focusDownButton->setEnabled(true);
  ui->expUpButton->setEnabled(true);
  ui->expDownButton->setEnabled(true);

  showingBackCamera = true;

  show();
}


void OreVideoMonitorForm::showFrontCamera()
{
  ui->lightButton->setEnabled(false);
  ui->focusUpButton->setEnabled(false);
  ui->focusDownButton->setEnabled(false);
  ui->expUpButton->setEnabled(false);
  ui->expDownButton->setEnabled(false);

  showingBackCamera = false;

  show();
}


void OreVideoMonitorForm::showVideo()
{
  // For now, this will be the same as showFrontCamera().
  showFrontCamera();
}


void OreVideoMonitorForm::on_lightButton_clicked()
{
  if (showingBackCamera)
  {
    camera.toggleTorch();
  }
}

void OreVideoMonitorForm::on_focusUpButton_pressed()
{
  // Only knw how to focus back camera:
  if (showingBackCamera)
  {
    camera.startIncrementingFocus();
  }
}

void OreVideoMonitorForm::on_focusUpButton_released()
{
  if (showingBackCamera)
  {
    camera.stopTimer();
  }
}

void OreVideoMonitorForm::on_focusDownButton_pressed()
{
  // Only knw how to focus back camera:
  if (showingBackCamera)
  {
    camera.startDecrementingFocus();
  }
}

void OreVideoMonitorForm::on_focusDownButton_released()
{
  if (showingBackCamera)
  {
    camera.stopTimer();
  }
}

void OreVideoMonitorForm::on_expUpButton_pressed()
{
  if (showingBackCamera)
  {
    camera.startIncrementingExposure();
  }
  else
  {
    camera.startIncrementingFrontExposure();
  }
}

void OreVideoMonitorForm::on_expUpButton_released()
{
  camera.stopTimer();
}

void OreVideoMonitorForm::on_expDownButton_pressed()
{
  if (showingBackCamera)
  {
    camera.startDecrementingExposure();
  }
  else
  {
    camera.startDecrementingFrontExposure();
  }
}

void OreVideoMonitorForm::on_expDownButton_released()
{
  camera.stopTimer();
}

void OreVideoMonitorForm::on_stopButton_clicked()
{
  mainWindow->on_stopButton_clicked();
  hide();

  // Don't force landscape any more:
  mainWindow->setAttribute(static_cast<Qt::WidgetAttribute>(129), false);
  // Turn auto orientation back on:
  mainWindow->setAttribute(static_cast<Qt::WidgetAttribute>(130), true);
}

void OreVideoMonitorForm::on_pauseButton_clicked()
{
  mainWindow->on_pauseButton_clicked();
}


void OreVideoMonitorForm::closeEvent(
  QCloseEvent *event)
{
  QWidget::closeEvent(event);

  mainWindow->on_stopButton_clicked();

  // Don't force landscape any more:
  mainWindow->setAttribute(static_cast<Qt::WidgetAttribute>(129), false);
  // Turn auto orientation back on:
  mainWindow->setAttribute(static_cast<Qt::WidgetAttribute>(130), true);
}
