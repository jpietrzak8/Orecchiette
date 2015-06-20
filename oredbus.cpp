//
// oredbus.cpp
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

#include "oredbus.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QStringList>
#include <QString>
#include <QtDebug>

#include <dbus/dbus.h>
#include <libosso.h>
//#include <libplayback/playback.h>


// Ugly Libplayback callbacks:

static void playbackStateReqCallback(
  pb_playback_t *pb,
  enum pb_state_e granted_state,
  const char *reason,
  pb_req_t *req,
  void *data)
{
  qWarning() << "State request callback, granted state: " << pb_state_to_string(granted_state);
  qWarning() << "Reason: " << reason;

  OreDBus *myDBus = static_cast<OreDBus *>(data);
  if (granted_state == PB_STATE_PLAY)
  {
    // OK to start playing
    myDBus->emitPlayingAllowed();
  }
  else
  {
    // Not ok to start playing
    myDBus->emitPlayingDenied();
  }

  pb_playback_req_completed(pb, req);
}


static void playbackStateReqHandler(
  pb_playback_t *pb,
  enum pb_state_e req_state,
  pb_req_t *ext_req,
  void *data)
{
  qWarning() << "External state change request to: " << pb_state_to_string(req_state);

  if (req_state == PB_STATE_STOP)
  {
    qWarning() << "Stopping playback (e.g., perhaps a call is incoming)";
    OreDBus *myDBus = static_cast<OreDBus *>(data);
    myDBus->emitPlayingDenied();
  }
}


OreDBus::OreDBus()
  : libplaybackPtr(0)
{
  // DBus signal for receiving or sending a call.  (Also notifies when
  // the user on this end terminates the call.)

  QDBusConnection::systemBus().connect(
    "com.nokia.csd.Call",
    "/com/nokia/csd/call",
    "com.nokia.csd.Call",
    "UserConnection",
    this,
    SLOT(phoneOffHook(bool)));

  // DBus signal when call is terminated from other end.
  QDBusConnection::systemBus().connect(
    "com.nokia.csd.Call",
    "/com/nokia/csd/call/1",
    "com.nokia.csd.Call",
    "Terminated",
    this,
    SLOT(phoneOnHook()));

/*
  if (!QDBusConnection::systemBus().connect(
    "com.nokia.csd.Call",
    "/com/nokia/csd/call",
    "com.nokia.csd.Call",
    "Coming",
    this,
    SLOT(testconnection(const QDBusObjectPath&, unsigned int))))
  {
    qDebug() << "Failed to make testing connection";
  }
*/

  // Initialize osso:
  osso_context_t *ossoContextPtr =
    osso_initialize("Orecchiette", "0.3.2", FALSE, NULL);

  // Initialize the libplaybackPtr:
  DBusConnection *rawSessionBus =
    (DBusConnection *)osso_get_dbus_connection(ossoContextPtr);

  libplaybackPtr = pb_playback_new_2(
    rawSessionBus,
    PB_CLASS_MEDIA, // MEDIA class allowed to play in silent mode
    PB_FLAG_VIDEO_RECORDING,
    PB_STATE_STOP,
    playbackStateReqHandler,
    this);

  pb_playback_set_stream(libplaybackPtr, "Playback Stream");
}


OreDBus::~OreDBus()
{
  if (libplaybackPtr)
  {
    pb_playback_destroy(libplaybackPtr);
  }
}


bool OreDBus::btDeviceInUse()
{
  QDBusInterface btProperties(
    "org.bluez",
    getBTPath(),
    "org.bluez.Adapter",
    QDBusConnection::systemBus());

  QDBusReply<QMap<QString, QVariant> > reply =
    btProperties.call("GetProperties");

//qDebug() << "Powered: " << reply.value()["Powered"].toBool();
  if (!reply.value()["Powered"].toBool())
  {
    // Bluetooth device is not turned on.
    return false;
  }

  QStringList deviceList = reply.value()["Devices"].toStringList();

  QStringList::const_iterator i = deviceList.begin();
  while (i != deviceList.end())
  {
    QDBusInterface btDevProperties(
      "org.bluez",
      *i,
      "org.bluez.Device",
      QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > devReply =
      btDevProperties.call("GetProperties");

//qDebug() << "Connected: " << devReply.value()["Connected"].toBool();;
    if (devReply.value()["Connected"].toBool())
    {
      QStringList uuidList = devReply.value()["UUIDs"].toStringList();

      QStringList::const_iterator ii = uuidList.begin();
      while (ii != uuidList.end())
      {
        if ( (ii->at(4) == '1')
          && (ii->at(5) == '1')
          && (ii->at(6) == '1')
          && ((ii->at(7) == 'e') || (ii->at(7) == 'E')))
        {
          // This is a hands-free device.
          return true;
        }

        ++ii;
      }
    }

    ++i;
  }

  // If we reach this point, we found no hands-free devices.
  return false;
}


void OreDBus::phoneOffHook(
  bool connection)
{
  if (!connection)
  {
    // User has hung up phone:
    emit callTerminated();
    return;
  }

  emit callStarted();
}


void OreDBus::phoneOnHook()
{
//qDebug() << "Phone on hook!";
  emit callTerminated();
}


QString OreDBus::getBTPath()
{
  QDBusInterface btDefaultAdapter(
    "org.bluez",
    "/",
    "org.bluez.Manager",
    QDBusConnection::systemBus());

  QDBusReply<QDBusObjectPath> reply = btDefaultAdapter.call("DefaultAdapter");

//  qDebug() << "Bluez path: " << reply.value().path();

  return reply.value().path();
}


/*
void OreDBus::testconnection(
  const QDBusObjectPath& path,
  unsigned int num)
{
  qDebug() << "Connection path: " << path.path();
  qDebug() << "Connection num: " << num;
}
*/


void OreDBus::requestToPlay()
{
  pb_playback_req_state(
    libplaybackPtr,
    PB_STATE_PLAY,
    playbackStateReqCallback,
    this);
}
