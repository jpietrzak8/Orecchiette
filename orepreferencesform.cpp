//
// orepreferencesform.cpp
//
// Implementation of the preferences form for Orecchiette
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

#include "orepreferencesform.h"
#include "ui_orepreferencesform.h"

#include <QFileDialog>
#include <QSettings>
#include <QDateTime>
#include <QStringList>
#include <QFile>
#include "orefilenameform.h"
#include "mainwindow.h"

#define FILE_NAME_FORMAT "FileNameFormat"

OrePreferencesForm::OrePreferencesForm(
  MainWindow *mw)
  : QWidget(mw),
    source(Microphone),
    audioDirectory("/home/user/MyDocs"),
    fileNameFormat("Recording %o"),
    recordPhoneAuthorized(false),
    startRecordingOnStartUp(false),
    unlimitedFileNumbers(false),
    maxFileNumber(20),
    nextFileNumber(1),
    mainWindow(mw),
    ui(new Ui::OrePreferencesForm)
{
  ui->setupUi(this);

  setAttribute(Qt::WA_Maemo5StackedWindow);
  setWindowFlags(windowFlags() | Qt::Window);

  // Setup the combo box:
//  ui->encodingComboBox->addItem("SPX", SPX_Encoding);
  ui->encodingComboBox->addItem("AAC", AAC_Encoding);
  ui->encodingComboBox->addItem("WAV", WAV_Encoding);
//  ui->encodingComboBox->addItem("FLAC", FLAC_Encoding);
//  ui->encodingComboBox->addItem("iLBC", ILBC_Encoding);

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
  }
  ui->currentDirectoryLabel->setText(audioDirectory);

  if (settings.contains("RecordPhoneAuthorized"))
  {
    recordPhoneAuthorized = settings.value("RecordPhoneAuthorized").toBool();
  }
  ui->recordPhoneCheckBox->setChecked(recordPhoneAuthorized);

  if (settings.contains("RecordOnStartUp"))
  {
    startRecordingOnStartUp = settings.value("RecordOnStartUp").toBool();
  }
  ui->recordOnStartCheckBox->setChecked(startRecordingOnStartUp);

  if (settings.contains("UnlimitedFileNumbers"))
  {
    unlimitedFileNumbers = settings.value("UnlimitedFileNumbers").toBool();
  }
  ui->unlimitedCheckBox->setChecked(unlimitedFileNumbers);

  if (settings.contains("MaxFileNumber"))
  {
    maxFileNumber = settings.value("MaxFileNumber").toInt();
  }
  ui->fileQuantitySpinBox->setValue(maxFileNumber);

  if (settings.contains("NextFileNumber"))
  {
    nextFileNumber = settings.value("NextFileNumber").toInt();
  }

//  fileNameFormat = settings.value(FILE_NAME_FORMAT, "Recording %o").toString();
  fileNameFormat = settings.value(FILE_NAME_FORMAT).toString();
  ui->currentFileNameFormatLabel->setText(fileNameFormat);

  if (settings.contains("Source"))
  {
    source = Source(settings.value("Source").toInt());
  }

  if (settings.contains("UsedFileNames"))
  {
    usedFileNamesList << settings.value("UsedFileNames").toStringList();
  }

  if (settings.contains("AACBitrateSet"))
  {
    ui->aacBitrateCheckBox->setChecked(
      settings.value("AACBitrateSet").toBool());
  }

  if (settings.contains("AACBitrateValue"))
  {
    ui->aacBitrateSpinBox->setValue(
      settings.value("AACBitrateValue").toInt());
  }

  if (settings.contains("AACOutputFormatSet"))
  {
    ui->aacOutputFormatCheckBox->setChecked(
      settings.value("AACOutputFormatSet").toBool());
  }

  if (settings.contains("AACOutputFormatValue"))
  {
    ui->aacOutputFormatSpinBox->setValue(
      settings.value("AACOutputFormatValue").toInt());
  }

  if (settings.contains("AACWidthSet"))
  {
    ui->aacWidthCheckBox->setChecked(
      settings.value("AACWidthSet").toBool());
  }

  if (settings.contains("AACWidthValue"))
  {
    ui->aacWidthSpinBox->setValue(
      settings.value("AACWidthValue").toInt());
  }

  if (settings.contains("AACDepthSet"))
  {
    ui->aacDepthCheckBox->setChecked(
      settings.value("AACDepthSet").toBool());
  }

  if (settings.contains("AACDepthValue"))
  {
    ui->aacDepthSpinBox->setValue(
      settings.value("AACDepthValue").toInt());
  }

  if (settings.contains("AACRateSet"))
  {
    ui->aacRateCheckBox->setChecked(
      settings.value("AACRateSet").toBool());
  }

  if (settings.contains("AACRateValue"))
  {
    ui->aacRateSpinBox->setValue(
      settings.value("AACRateValue").toInt());
  }

  if (settings.contains("AACChannelsSet"))
  {
    ui->aacChannelsCheckBox->setChecked(
      settings.value("AACChannelsSet").toBool());
  }

  if (settings.contains("AACChannelsValue"))
  {
    ui->aacChannelsSpinBox->setValue(
      settings.value("AACChannelsValue").toInt());
  }
}


OrePreferencesForm::~OrePreferencesForm()
{
  QSettings settings("pietrzak.org", "Orecchiette");

  settings.setValue("AudioEncoding", getEncoding());
  settings.setValue("AudioDirectory", audioDirectory);
  settings.setValue(FILE_NAME_FORMAT, fileNameFormat);
  settings.setValue("RecordOnStartUp", startRecordingOnStartUp);
  settings.setValue("RecordPhoneAuthorized", recordPhoneAuthorized);
  settings.setValue("UnlimitedFileNumbers", unlimitedFileNumbers);
  settings.setValue("MaxFileNumber", maxFileNumber);
  settings.setValue("NextFileNumber", nextFileNumber);
  settings.setValue("Source", int(source));
  settings.setValue("UsedFileNames", usedFileNamesList);

  settings.setValue("AACBitrateSet", ui->aacBitrateCheckBox->isChecked());
  settings.setValue("AACBitrateValue", ui->aacBitrateSpinBox->value());

  settings.setValue(
    "AACOutputFormatSet",
    ui->aacOutputFormatCheckBox->isChecked());
  settings.setValue(
    "AACOutputFormatValue",
    ui->aacOutputFormatSpinBox->value());

  settings.setValue("AACWidthSet", ui->aacWidthCheckBox->isChecked());
  settings.setValue("AACWidthValue", ui->aacWidthSpinBox->value());
  settings.setValue("AACDepthSet", ui->aacDepthCheckBox->isChecked());
  settings.setValue("AACDepthValue", ui->aacDepthSpinBox->value());
  settings.setValue("AACRateSet", ui->aacRateCheckBox->isChecked());
  settings.setValue("AACRateValue", ui->aacRateSpinBox->value());
  settings.setValue("AACChannelsSet", ui->aacChannelsCheckBox->isChecked());
  settings.setValue("AACChannelsValue", ui->aacChannelsSpinBox->value());
 
  delete ui;
}


QString OrePreferencesForm::resolveFilename(
  const QString &formatString)
{
  QString nextFilename;
  const QDateTime date = QDateTime::currentDateTime();

  for (QString::ConstIterator it = formatString.constBegin();
       it != formatString.constEnd();
       ++it)
  {
    const QChar c = *it;
    if (c != '%')
    {
      nextFilename.append(c);
      continue;  // move along, nothing to see here
    }

    if (++it == formatString.constEnd())
    {
      // Current char is '%', but we've hit the end of the string
      nextFilename.append(c);
      break;    // we might as well bail out here
    }

    const QChar d = *it;
    switch(d.unicode())
    {
    case '%':   // %%, The % character
      nextFilename.append(d);
      break;
    case 'o':   // %o, Recording number (integer, always increasing)
      nextFilename.append(QString::number(nextFileNumber));
      break;
    case 'O':   // %O, Recording number, 6 digits (000001, 000002...)
      nextFilename.append(QString::number(nextFileNumber).rightJustified(6, '0'));
      break;
    case 'q':   // %q, Caller's number, if available (441234567890)
    case 'Q':   // %Q, As %q but "unknown" if number not available
      break;
    case 'a':   // %a, Weekday, abbreviated (Mon, Tue, Wed...)
      nextFilename.append(date.toString("ddd"));
      break;
    case 'A':   // %A, Weekday, full (Monday, Tuesday, Wednesday...)
      nextFilename.append(date.toString("dddd"));
      break;
    case 'd':   // %d, Day of the month, 2 digits (01-31)
      nextFilename.append(date.toString("dd"));
      break;
    case 'e':   // %e, Day of the month (1-31)
      nextFilename.append(date.toString("d"));
      break;
    case 'm':   // %m, Month number, 2 digits (01-12)
      nextFilename.append(date.toString("MM"));
      break;
    case 'b':   // %b, Month name, abbreviated (Jan, Feb, Mar...)
      nextFilename.append(date.toString("MMM"));
      break;
    case 'B':   // %B, Month name, full (January, February, March...)
      nextFilename.append(date.toString("MMMM"));
      break;
    case 'y':   // %y, Year, 2 digits (00-99)
      nextFilename.append(date.toString("yy"));
      break;
    case 'Y':   // %Y, Year, 4 digits (1900-2100)
      nextFilename.append(date.toString("yyyy"));
      break;
    case 'l':   // %l, Hour, 12h (1-12)
      nextFilename.append(date.toString("h ap").section(' ', 0, 0));
      break;
    case 'I':   // %I, Hour, 12h, 2 digits (01-12)
      nextFilename.append(date.toString("hh ap").section(' ', 0, 0));
      break;
    case 'k':   // %k, Hour, 24h (0-23)
      nextFilename.append(date.toString("h"));
      break;
    case 'H':   // %H, Hour, 24h, 2 digits (00-23)
      nextFilename.append(date.toString("hh"));
      break;
    case 'p':   // %p, AM or PM, uppercase
      nextFilename.append(date.toString("AP"));
      break;
    case 'P':   // %P, am or pm, lovercase
      nextFilename.append(date.toString("ap"));
      break;
    case 'M':   // %M, Minute, 2 digits (00-59)
      nextFilename.append(date.toString("mm"));
      break;
    case 's':   // %s, Seconds since 00:00:00 1970-01-01 UTC
      nextFilename.append(QString::number(date.toTime_t()));
      break;
    case 'S':   // %S, Second, 2 digits (00-59)
      nextFilename.append(date.toString("ss"));
      break;
    default:    // any other character, just skip
      break;
    }
  }

  return nextFilename;
}


QString OrePreferencesForm::getNextFilename()
{
  QString nextFilename = audioDirectory;
  nextFilename.append('/');
  nextFilename.append(resolveFilename(fileNameFormat));
  nextFilename.append(getEncodingExtension());

  ++nextFileNumber;
  usedFileNamesList.append(nextFilename);
  while (usedFileNamesList.size() > maxFileNumber)
  {
    const QString fn = usedFileNamesList.takeFirst();
    if (!unlimitedFileNumbers)
    {
      QFile::remove(fn);
    }
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


bool OrePreferencesForm::recordOnStartUp()
{
  return startRecordingOnStartUp;
}


AudioEncoding OrePreferencesForm::getEncoding()
{
  return AudioEncoding(ui->encodingComboBox->itemData(
    ui->encodingComboBox->currentIndex()).toInt());
}


QString OrePreferencesForm::getEncodingExtension()
{
  if (mainWindow->recordingVideo())
  {
    // For now, only using Matroska for video container:
    return ".mkv";
  }

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

  case ILBC_Encoding:
    return ".lbc";

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


void OrePreferencesForm::on_editFileNameFormatButton_clicked()
{
  OreFileNameForm form(this, fileNameFormat);
  form.exec();
  ui->currentFileNameFormatLabel->setText(fileNameFormat);
}


void OrePreferencesForm::on_recordPhoneCheckBox_toggled(
  bool checked)
{
  recordPhoneAuthorized = checked;
}


void OrePreferencesForm::on_recordOnStartCheckBox_toggled(
  bool checked)
{
  startRecordingOnStartUp = checked;
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


void OrePreferencesForm::on_encodingComboBox_currentIndexChanged(
  int index)
{
  emit encodingChanged(
    AudioEncoding(
      ui->encodingComboBox->itemData(index).toInt()));
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


bool OrePreferencesForm::aacBitrateSet() const
{
  return ui->aacBitrateCheckBox->isChecked();
}

int OrePreferencesForm::aacBitrateValue() const
{
  return ui->aacBitrateSpinBox->value();
}

bool OrePreferencesForm::aacOutputFormatSet() const
{
  return ui->aacOutputFormatCheckBox->isChecked();
}

int OrePreferencesForm::aacOutputFormatValue() const
{
  return ui->aacOutputFormatSpinBox->value();
}

bool OrePreferencesForm::aacWidthSet() const
{
  return ui->aacWidthCheckBox->isChecked();
}

int OrePreferencesForm::aacWidthValue() const
{
  return ui->aacWidthSpinBox->value();
}

bool OrePreferencesForm::aacDepthSet() const
{
  return ui->aacDepthCheckBox->isChecked();
}

int OrePreferencesForm::aacDepthValue() const
{
  return ui->aacDepthSpinBox->value();
}

bool OrePreferencesForm::aacRateSet() const
{
  return ui->aacRateCheckBox->isChecked();
}

int OrePreferencesForm::aacRateValue() const
{
  return ui->aacRateSpinBox->value();
}

bool OrePreferencesForm::aacChannelsSet() const
{
  return ui->aacChannelsCheckBox->isChecked();
}

int OrePreferencesForm::aacChannelsValue() const
{
  return ui->aacChannelsSpinBox->value();
}
