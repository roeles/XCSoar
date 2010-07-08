/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/Fonts.hpp"
#include "LogFile.hpp"
#include "UtilsFont.hpp"
#include "InfoBoxLayout.hpp"
#include "ButtonLabel.hpp"
#include "Profile.hpp"
#include "Screen/Layout.hpp"
#include "Screen/VirtualCanvas.hpp"
#include "Appearance.hpp"
#include "InfoBoxLayout.hpp"

#include <stdio.h>

/// values inside infoboxes  like numbers, etc.
Font InfoWindowFont;
Font InfoWindowSmallFont;
/// Titles of infoboxes like Next, WP L/D etc.
Font TitleWindowFont;
/// text names on the map
Font MapWindowFont;
/// menu buttons, waypoint selection, messages, etc.
Font MapWindowBoldFont;
/// vario display, runway informations
Font CDIWindowFont; // New
/// Flarm Traffic draweing and stats, map labels in italic
Font MapLabelFont;
Font StatisticsFont;

// these are the non-custom parameters
LOGFONT InfoWindowLogFont;
LOGFONT TitleWindowLogFont;
LOGFONT MapWindowLogFont;
LOGFONT InfoWindowSmallLogFont;
LOGFONT MapWindowBoldLogFont;
LOGFONT CDIWindowLogFont;
LOGFONT MapLabelLogFont;
LOGFONT StatisticsLogFont;

#ifndef ENABLE_SDL

static bool
IsNullLogFont(LOGFONT logfont)
{
  LOGFONT LogFontBlank;
  memset((char *)&LogFontBlank, 0, sizeof(LOGFONT));

  if (memcmp(&logfont, &LogFontBlank, sizeof(LOGFONT)) == 0)
    return true;

  return false;
}

#endif /* !ENABLE_SDL */

static void
InitialiseLogfont(LOGFONT* font, const TCHAR* facename, bool variable_pitch,
                  int height, bool bold, bool italic)
{
#ifndef ENABLE_SDL
  memset((char *)font, 0, sizeof(LOGFONT));

  _tcscpy(font->lfFaceName, facename);
  font->lfPitchAndFamily = (variable_pitch ? VARIABLE_PITCH : FIXED_PITCH)
                          | FF_DONTCARE;
  font->lfHeight = (long)height;
  font->lfWeight = (long)(bold ? FW_BOLD : FW_MEDIUM);
  font->lfItalic = italic;
  font->lfQuality = ANTIALIASED_QUALITY;
#endif /* !ENABLE_SDL */
}

void
InitializeFont(Font *theFont, LOGFONT autoLogFont,
                  LOGFONT * LogFontUsed)
{
#ifdef ENABLE_SDL
  // XXX
#else /* !ENABLE_SDL */
  if (theFont->defined() || IsNullLogFont(autoLogFont))
    return;

  if (theFont->set(&autoLogFont) && LogFontUsed != NULL)
    *LogFontUsed = autoLogFont; // RLD save for custom font GUI
#endif /* !ENABLE_SDL */
}

void
LoadCustomFont(Font *theFont, const TCHAR FontRegKey[], LOGFONT * LogFontUsed)
{
#ifdef ENABLE_SDL
  // XXX
#else /* !ENABLE_SDL */
  LOGFONT logfont;
  memset((char *)&logfont, 0, sizeof(LOGFONT));
  if (!Profile::GetFont(FontRegKey, &logfont))
    return;

  if (theFont->set(&logfont) && LogFontUsed != NULL)
    *LogFontUsed = logfont; // RLD save for custom font GUI
#endif /* !ENABLE_SDL */
}

static void
InitialiseFontsAltair()
{
  if (!is_altair())
    return;

  InitialiseLogfont(&InfoWindowLogFont, _T("RasterGothicTwentyFourCond"),
                    true, 24, true, false);
  InitialiseLogfont(&TitleWindowLogFont, _T("RasterGothicNineCond"),
                    true, 10, false, false);
  InitialiseLogfont(&CDIWindowLogFont, _T("RasterGothicEighteenCond"),
                    true, 19, true, false);
  InitialiseLogfont(&MapLabelLogFont, _T("RasterGothicTwelveCond"),
                    true, 13, false, false);
  InitialiseLogfont(&StatisticsLogFont, _T("RasterGothicFourteenCond"),
                    true, 15, false, false);
  InitialiseLogfont(&MapWindowLogFont, _T("RasterGothicFourteenCond"),
                    true, 15, false, false);
  InitialiseLogfont(&MapWindowBoldLogFont, _T("RasterGothicFourteenCond"),
                    true, 15, true, false);
  InitialiseLogfont(&InfoWindowSmallLogFont, _T("RasterGothicEighteenCond"),
                    true, 19, true, false);
}

static void
InitialiseLogFonts()
{
  int FontHeight = (Layout::square ? Layout::Scale(26) : Layout::Scale(35));

#ifdef WINDOWSPC
  FontHeight = (int)(FontHeight / 1.35);
#endif

#ifndef ENABLE_SDL
  // oversize first so can then scale down
  int iFontHeight = (int)(FontHeight * 1.4);

  LOGFONT logfont;
  InitialiseLogfont(&logfont, _T("Tahoma"),
                    true, iFontHeight, true, false);
  logfont.lfCharSet = ANSI_CHARSET;

  // JMW algorithm to auto-size info window font.
  // this is still required in case title font property doesn't exist.
  VirtualCanvas canvas(1, 1);
  SIZE tsize;
  do {
    HFONT TempWindowFont;
    HFONT hfOld;

    iFontHeight--;
    logfont.lfHeight = iFontHeight;

    TempWindowFont = CreateFontIndirect(&logfont);
    hfOld = (HFONT)SelectObject(canvas, TempWindowFont);

    tsize = canvas.text_size(_T("1234m"));
    // unselect it before deleting it
    SelectObject(canvas, hfOld);
    DeleteObject(TempWindowFont);
  } while (tsize.cx > InfoBoxLayout::ControlWidth);

  iFontHeight++;
  logfont.lfHeight = iFontHeight;
  memset(&InfoWindowLogFont, 0, sizeof(LOGFONT));
  memcpy(&InfoWindowLogFont, &logfont, sizeof(LOGFONT));

#else /* !ENABLE_SDL */
  // XXX implement
#endif /* !ENABLE_SDL */

  InitialiseLogfont(&TitleWindowLogFont, _T("Tahoma"), true,
                    (int)(FontHeight * 0.333), true, false);

  // new font for CDI Scale
  InitialiseLogfont(&CDIWindowLogFont, _T("Tahoma"), false,
                    (int)(FontHeight * 0.6), false, false);

  // new font for map labels
  InitialiseLogfont(&MapLabelLogFont, _T("Tahoma"), true,
                    (int)(FontHeight * 0.39), false, true);

  // Font for map other text
  InitialiseLogfont(&StatisticsLogFont, _T("Tahoma"), true,
                    (int)(FontHeight * 0.7), false, false);

  // new font for map labels
  InitialiseLogfont(&MapWindowLogFont, _T("Tahoma"), true,
                    (int)(FontHeight * 0.507), false, false);

  // Font for map bold text
  InitialiseLogfont(&MapWindowBoldLogFont, _T("Tahoma"), true,
                    (int)(FontHeight * 0.507), true, false);

  InitialiseLogfont(&InfoWindowSmallLogFont, _T("Tahoma"), true,
                    Layout::Scale(20), false, false);
}

void
InitialiseFonts(const struct Appearance &appearance, RECT rc)
{
  //this routine must be called only at start/restart of XCSoar b/c there are many pointers to these fonts
  ResetFonts();

  InitialiseLogFonts();

  InitialiseFontsAltair();

  InitializeFont(&InfoWindowFont, InfoWindowLogFont);
  InitializeFont(&InfoWindowSmallFont, InfoWindowSmallLogFont);
  InitializeFont(&TitleWindowFont, TitleWindowLogFont);
  InitializeFont(&CDIWindowFont, CDIWindowLogFont);
  InitializeFont(&MapLabelFont, MapLabelLogFont);
  InitializeFont(&StatisticsFont, StatisticsLogFont);
  InitializeFont(&MapWindowFont, MapWindowLogFont);
  InitializeFont(&MapWindowBoldFont, MapWindowBoldLogFont);

  if (appearance.UseCustomFonts) {
    LoadCustomFont(&InfoWindowFont, szProfileFontInfoWindowFont);
    LoadCustomFont(&InfoWindowSmallFont, szProfileFontTitleSmallWindowFont);
    LoadCustomFont(&TitleWindowFont, szProfileFontTitleWindowFont);
    LoadCustomFont(&CDIWindowFont, szProfileFontCDIWindowFont);
    LoadCustomFont(&MapLabelFont, szProfileFontMapLabelFont);
    LoadCustomFont(&StatisticsFont, szProfileFontStatisticsFont);
    LoadCustomFont(&MapWindowFont, szProfileFontMapWindowFont);
    LoadCustomFont(&MapWindowBoldFont, szProfileFontMapWindowBoldFont);
  }
}

void
ResetFonts()
{
  InfoWindowFont.reset();
  InfoWindowSmallFont.reset();
  TitleWindowFont.reset();
  CDIWindowFont.reset();
  MapLabelFont.reset();
  MapWindowFont.reset();
  MapWindowBoldFont.reset();
  StatisticsFont.reset();
}
