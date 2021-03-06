//
// oreencoding.h
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

#ifndef OREENCODING_H
#define OREENCODING_H

enum AudioEncoding
{
  SPX_Encoding,
  AAC_Encoding,
  WAV_Encoding,
  FLAC_Encoding,
  ILBC_Encoding
};

enum OreAudioSource
{
  No_Audio,
  Microphone_Audio,
  Speaker_Audio,
  MicrophoneAndSpeaker_Audio
};

enum OreVideoSource
{
  No_Video,
  Screen_Video,
  BackCamera_Video,
  FrontCamera_Video
//  MJpegStream_Video
};

enum OreAudioPosition
{
  Mic_Center,
  Mic_Left,
  Mic_Right
};

#endif // OREENCODING_H
