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

#include "Terrain/RasterMap.hpp"
#include "Math/Earth.hpp"

// Rounding control


bool
RasterMap::GetMapCenter(GEOPOINT *loc) const
{
  if(!isMapLoaded())
    return false;

  *loc = TerrainInfo.TopLeft.interpolate(TerrainInfo.BottomRight,
                                         fixed_half);
  return true;
}



// accurate method
int
RasterMap::GetEffectivePixelSize(fixed &pixel_D,
                                 const GEOPOINT &location) const
{
  fixed terrain_step_x, terrain_step_y;
  Angle step_size = TerrainInfo.StepSize * sqrt(fixed_two); 

  if (negative(pixel_D) || (step_size.sign()==0)) {
    pixel_D = fixed_one;
    return 1;
  }
  GEOPOINT dloc;
  
  // how many steps are in the pixel size
  dloc = location; dloc.Latitude += step_size;
  terrain_step_x = Distance(location, dloc);

  dloc = location; dloc.Longitude += step_size;
  terrain_step_y = Distance(location, dloc);

  fixed rfact = max(terrain_step_x, terrain_step_y) / pixel_D;

  int epx = (int)(max(fixed_one, ceil(rfact)));
  //  *pixel_D = (*pixel_D)*rfact/epx;
  return epx;
}

// JMW rounding further reduces data as required to speed up terrain
// display on low zoom levels
short
RasterMap::GetField(const GEOPOINT &location)
{
  if(isMapLoaded()) {
    return _GetFieldAtXY((int)(location.Longitude.value_native() *
                               rounding.fXroundingFine) - rounding.xlleft,
                         rounding.xlltop -
                         (int)(location.Latitude.value_native() *
                               rounding.fYroundingFine));
  } else {
    return TERRAIN_INVALID;
  }
}
