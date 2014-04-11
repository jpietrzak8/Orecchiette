//
// orefilenameform.cpp
//
// Implementation of the filename dialog for Orecchiette
//
// Copyright 2014 by John Pietrzak (jpietrzak8@gmail.com)
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

#include "orefilenameform.h"
#include "ui_orefilenameform.h"

#include "orepreferencesform.h"


OreFileNameForm::OreFileNameForm(
  QWidget *parent,
  QString &format)
  : QDialog(parent),
    fileNameFormat(format),
    ui(new Ui::OreFileNameForm)
{
  ui->setupUi(this);

  ui->fileNameFormatLineEdit->setText(fileNameFormat);
  ui->pushButtonValidate->click();
}


OreFileNameForm::~OreFileNameForm()
{
  delete ui;
}


void OreFileNameForm::on_formatSpecifierComboBox_activated(
  const QString & text)
{
  ui->fileNameFormatLineEdit->insert(text.left(2));
}


void OreFileNameForm::on_pushButtonValidate_clicked()
{
  OrePreferencesForm *prefs = (OrePreferencesForm *)parent();
  QString temp = ui->fileNameFormatLineEdit->text();
  temp = prefs->resolveFilename(temp);
  ui->validatedFileNameLabel->setText(temp);
}


void OreFileNameForm::on_pushButtonOK_clicked()
{
  fileNameFormat = ui->fileNameFormatLineEdit->text();
  accept();
}
