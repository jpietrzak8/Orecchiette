//
// orepreferencesform.h
//
// Implementation of the "Preferences" window for Orecchiette.
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

#ifndef OREPREFERENCESFORM_H
#define OREPREFERENCESFORM_H

#include <QWidget>

#include "oreencoding.h"

namespace Ui {
class OrePreferencesForm;
}

class OrePreferencesForm : public QWidget
{
  Q_OBJECT
  
public:
  explicit OrePreferencesForm(QWidget *parent = 0);
  ~OrePreferencesForm();

  QString getNextFilename();

  QString getAudioDirectory();

  bool recordPhoneCalls();

signals:
  void encodingChanged(
    AudioEncoding ae);
  
private slots:
  void on_chooseDirectoryButton_clicked();
  void on_recordPhoneCheckBox_toggled(bool checked);
  void on_unlimitedCheckBox_toggled(bool checked);
  void on_fileQuantitySpinBox_valueChanged(int arg1);

  void on_encodingComboBox_currentIndexChanged(int index);

private:
  AudioEncoding getEncoding();

  void setEncoding(
    AudioEncoding enc);

  QString getEncodingExtension();

  QString audioDirectory;
  bool recordPhoneAuthorized;
  bool unlimitedFileNumbers;
  unsigned int maxFileNumber;
  unsigned int nextFileNumber;

  Ui::OrePreferencesForm *ui;
};

#endif // OREPREFERENCESFORM_H
