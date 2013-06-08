//
// orepreferencesform.cpp
//
// Implementation of the "About" window for Pierogi.
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

#include "orepreferencesform.h"
#include "ui_orepreferencesform.h"

#include <QFileDialog>
#include <QSettings>

OrePreferencesForm::OrePreferencesForm(
  QWidget *parent)
  : QWidget(parent),
    audioDirectory("/home/user/MyDocs"),
    recordPhoneAuthorized(false),
    unlimitedFileNumbers(false),
    maxFileNumber(20),
    nextFileNumber(1),
    ui(new Ui::OrePreferencesForm)
{
  ui->setupUi(this);

  setAttribute(Qt::WA_Maemo5StackedWindow);
  setWindowFlags(windowFlags() | Qt::Window);

  // Setup the combo box:
  ui->encodingComboBox->addItem("SPX", SPX_Encoding);
  ui->encodingComboBox->addItem("AAC", AAC_Encoding);
  ui->encodingComboBox->addItem("WAV", WAV_Encoding);
  ui->encodingComboBox->addItem("FLAC", FLAC_Encoding);

  QSettings settings("pietrzak.org", "Orecchiette");

  if (settings.contains("AudioEncoding"))
  {
    setEncoding(
      AudioEncoding(
        settings.value("AudioEncoding").toInt()));
  }

  if (settings.contains("AudioDirectory"))
  {
    audioDirectory = settings.value("AudioDirectory").toString();
    ui->currentDirectoryLabel->setText(audioDirectory);
  }

  if (settings.contains("RecordPhoneAuthorized"))
  {
    recordPhoneAuthorized = settings.value("RecordPhoneAuthorized").toBool();
    ui->recordPhoneCheckBox->setChecked(recordPhoneAuthorized);
  }

  if (settings.contains("UnlimitedFileNumbers"))
  {
    unlimitedFileNumbers = settings.value("UnlimitedFileNumbers").toBool();
    ui->unlimitedCheckBox->setChecked(unlimitedFileNumbers);
  }

  if (settings.contains("MaxFileNumber"))
  {
    maxFileNumber = settings.value("MaxFileNumber").toInt();
    ui->fileQuantitySpinBox->setValue(maxFileNumber);
  }

  if (settings.contains("NextFileNumber"))
  {
    nextFileNumber = settings.value("NextFileNumber").toInt();
  }
}


OrePreferencesForm::~OrePreferencesForm()
{
  QSettings settings("pietrzak.org", "Orecchiette");

  settings.setValue("AudioEncoding", getEncoding());
  settings.setValue("AudioDirectory", audioDirectory);
  settings.setValue("RecordPhoneAuthorized", recordPhoneAuthorized);
  settings.setValue("UnlimitedFileNumbers", unlimitedFileNumbers);
  settings.setValue("MaxFileNumber", maxFileNumber);
  settings.setValue("NextFileNumber", nextFileNumber);
 
  delete ui;
}


QString OrePreferencesForm::getNextFilename()
{
  QString nextFilename = audioDirectory;

  nextFilename.append("/Recording_");

  nextFilename.append(QString::number(nextFileNumber));

  nextFilename.append(getEncodingExtension());

  if (!unlimitedFileNumbers && (nextFileNumber >= maxFileNumber))
  {
    nextFileNumber = 1;
  }
  else
  {
    ++nextFileNumber;
  }

  return nextFilename;
}


QString OrePreferencesForm::getAudioDirectory()
{
  return audioDirectory;
}


bool OrePreferencesForm::recordPhoneCalls()
{
  return recordPhoneAuthorized;
}


AudioEncoding OrePreferencesForm::getEncoding()
{
  return AudioEncoding(ui->encodingComboBox->itemData(
    ui->encodingComboBox->currentIndex()).toInt());
}


QString OrePreferencesForm::getEncodingExtension()
{
  switch (getEncoding())
  {
  case SPX_Encoding:
    return ".spx";

  case AAC_Encoding:
    return ".aac";

  case WAV_Encoding:
    return ".wav";

  case FLAC_Encoding:
    return ".flac";

  default:
    // Should we throw an error here?
    return "";
  }
}


void OrePreferencesForm::on_chooseDirectoryButton_clicked()
{
  QString dirname = QFileDialog::getExistingDirectory(
    this,
    "Choose Audio Directory",
    "/home/user/MyDocs");

  // Make sure the user actually chose a directory:
  if (dirname.isEmpty()) return;

  audioDirectory = dirname;

  ui->currentDirectoryLabel->setText(audioDirectory);
}


void OrePreferencesForm::on_recordPhoneCheckBox_toggled(
  bool checked)
{
  recordPhoneAuthorized = checked;
}


void OrePreferencesForm::setEncoding(
  AudioEncoding enc)
{
  int index = ui->encodingComboBox->findData(enc);

  if (index != -1)
  {
    ui->encodingComboBox->setCurrentIndex(index);
  }
}


void OrePreferencesForm::on_unlimitedCheckBox_toggled(
  bool checked)
{
  unlimitedFileNumbers = checked;
}


void OrePreferencesForm::on_fileQuantitySpinBox_valueChanged(
  int arg1)
{
  maxFileNumber = arg1;
}
