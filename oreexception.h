//
// oreexception.h
//
// Copyright 2013 by John Pietrzak  (jpietrzak8@gmail.com)
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

#ifndef OREEXCEPTION_H
#define OREEXCEPTION_H

#include <QString>

class OreException
{
public:
  OreException(
    QString s)
  : errStr(s)
  {}

  QString getError();

  void display();

private:
  QString errStr;
};


inline QString OreException::getError()
{
  return errStr;
}

#endif // OREEXCEPTION_H
