//
// oremjpegdialog.cpp
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

#include "oremjpegdialog.h"
#include "ui_oremjpegdialog.h"

#include <QSettings>

OreMJpegDialog::OreMJpegDialog(
  QWidget *parent)
  : QDialog(parent),
    ui(new Ui::OreMJpegDialog)
{
  ui->setupUi(this);

  QSettings settings("pietrzak.org", "Orecchiette");

  if (settings.contains("MJpegUrl"))
  {
    ui->urlLineEdit->setText(settings.value("MJpegUrl").toString());
  }
}


OreMJpegDialog::~OreMJpegDialog()
{
  QSettings settings("pietrzak.org", "Orecchiette");

  settings.setValue("MJpegUrl", ui->urlLineEdit->text());

  delete ui;
}


QString OreMJpegDialog::getUrl()
{
  return ui->urlLineEdit->text();
}
