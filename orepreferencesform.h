//
// orepreferencesform.h
//
// Implementation of the "Preferences" window for Orecchiette.
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

#ifndef OREPREFERENCESFORM_H
#define OREPREFERENCESFORM_H

#include <QWidget>

#include "oreencoding.h"

class MainWindow;

namespace Ui {
class OrePreferencesForm;
}

class OrePreferencesForm : public QWidget
{
  Q_OBJECT
  
public:
  OrePreferencesForm(MainWindow *mw);
  ~OrePreferencesForm();

  QString resolveFilename(const QString &format);
  QString getNextFilename();
  QString getAudioDirectory();

  bool recordPhoneCalls();
  bool recordOnStartUp();

  enum Source {
    Microphone, Speaker, Both
  } source; // deliberately public

signals:
  void encodingChanged(
    AudioEncoding ae);
  
private slots:
  void on_chooseDirectoryButton_clicked();
  void on_editFileNameFormatButton_clicked();
  void on_recordPhoneCheckBox_toggled(bool checked);
  void on_recordOnStartCheckBox_toggled(bool checked);
  void on_unlimitedCheckBox_toggled(bool checked);
  void on_fileQuantitySpinBox_valueChanged(int arg1);

  void on_encodingComboBox_currentIndexChanged(int index);

private:
  AudioEncoding getEncoding();

  void setEncoding(
    AudioEncoding enc);

  QString getEncodingExtension();

  QString audioDirectory;
  QString fileNameFormat;
  bool recordPhoneAuthorized;
  bool startRecordingOnStartUp;
  bool unlimitedFileNumbers;
  int maxFileNumber;
  int nextFileNumber;
  QStringList usedFileNamesList;

  MainWindow *mainWindow;

  Ui::OrePreferencesForm *ui;
};

#endif // OREPREFERENCESFORM_H
