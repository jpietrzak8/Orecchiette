//
// oreaboutform.h
//
// Implementation of the "About" window for Orecchiette.
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
#ifndef OREABOUTFORM_H
#define OREABOUTFORM_H

#include <QWidget>

namespace Ui {
class OreAboutForm;
}

class OreAboutForm : public QWidget
{
  Q_OBJECT
  
public:
  explicit OreAboutForm(QWidget *parent = 0);
  ~OreAboutForm();
  
private:
  Ui::OreAboutForm *ui;
};

#endif // OREABOUTFORM_H
