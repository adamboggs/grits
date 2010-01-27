/*
 * Copyright (C) 2009-2010 Andy Spencer <andy753421@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <math.h>

#include "gis-util.h"

/******************
 * Global helpers *
 ******************/
void lle2xyz(gdouble lat, gdouble lon, gdouble elev,
		gdouble *x, gdouble *y, gdouble *z)
{
	gdouble rad  = elev2rad(elev);
	gdouble azim = lon2azim(lon);
	gdouble incl = lat2incl(lat);
	*z = rad * cos(azim) * sin(incl);
	*x = rad * sin(azim) * sin(incl);
	*y = rad * cos(incl);
}

void xyz2lle(gdouble x, gdouble y, gdouble z,
		gdouble *lat, gdouble *lon, gdouble *elev)
{
	gdouble rad = sqrt(x*x + y*y + z*z);
	*lat  = incl2lat(acos(y / rad));
	*lon  = azim2lon(atan2(x,z));
	*elev = rad2elev(rad);
}

void xyz2ll(gdouble x, gdouble y, gdouble z,
		gdouble *lat, gdouble *lon)
{
	gdouble rad = sqrt(x*x + y*y + z*z);
	*lat = incl2lat(acos(y / rad));
	*lon = azim2lon(atan2(x,z));
}

gdouble ll2m(gdouble lon_dist, gdouble lat)
{
	gdouble azim = (-lat+90)/180*M_PI;
	gdouble rad  = sin(azim) * EARTH_R;
	gdouble circ = 2 * M_PI * rad;
	return lon_dist/360 * circ;
}

gdouble distd(gdouble *a, gdouble *b)
{
	return sqrt((a[0]-b[0])*(a[0]-b[0]) +
	            (a[1]-b[1])*(a[1]-b[1]) +
	            (a[2]-b[2])*(a[2]-b[2]));
}

gdouble lon_avg(gdouble a, gdouble b)
{
	gdouble diff = ABS(a-b);
	gdouble avg  = (a+b)/2;
	if (diff > 180) {
		if (avg >= 0)
			avg -= 180;
		else
			avg += 180;
	}
	return avg;
}
