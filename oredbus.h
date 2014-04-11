//
// oredbus.h
//
// Copyright 2013 by John Pietrzak  (jpietrzak8@gmail.com)
//
// This file is part of Orecchette.
//
// Orecchette is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Orecchette is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Orecchette; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef OREDBUS_H
#define OREDBUS_H

#include <QObject>

class QDBusMessage;
class QDBusObjectPath;

class OreDBus: public QObject
{
  Q_OBJECT

public:
  OreDBus();
  ~OreDBus();

  bool btDeviceInUse();

signals:
  void callStarted();
  void callTerminated();

public slots:
  void phoneOffHook(
    bool connection);

  void phoneOnHook();

/*
  void testconnection(
    const QDBusObjectPath &path,
    unsigned int num);
*/

private:
  QString getBTPath();
};

#endif // OREDBUS_H
