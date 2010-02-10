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

/**
 * SECTION:gis-util
 * @short_description: Geographic utilities
 *
 * Miscellaneous utility functions, these deal mostly with coordinate
 * conversion. Below are some examples that should help demonstrate how these
 * functions work.
 *
 * <example>
 * <title>Terminology</title>
 * <programlisting>
 * deg    - Degrees
 * rad    - Radians, also radius
 * m      - Meters, for earth-based distances
 * px     - Pixels, for screen-based distances
 *
 * height - Height, the distance above the geoid (ground)
 * elev   - Elevation, the distance above the spheroid
 * rad    - Radius, the distance from the center of the earth
 *
 * lat    - Latitude, amount north-south, -90 (S) .. 90 (N)
 * lon    - Longitude, amount east-west, -180 (W) .. 180 (E)
 * incl   - Inclination, polar equiv of  latitude, Pi .. 0
 * azim   - Azimuth, polar equiv of longitude, -Pi .. Pi
 *
 * x      -  0° lon is positive
 * y      - 90° lon is positive
 * z      - North pole is positive
 *
 * llh    - lat,lon,height
 * lle    - lat,lon,elev
 * llr    - lat,lon,rad
 * pol    - incl,azim,rad
 * xyz    - x,y,z
 * </programlisting>
 * </example>
 *
 * <example>
 * <title>Conversions</title>
 * <programlisting>
 *             lat    lon   elev ->      x      y      z
 * lle2xyz:    0.0,   0.0,   0.0 ->    0.0,   0.0,  10.0
 * lle2xyz:   90.0,   0.0,   0.0 ->    0.0,  10.0,   0.0
 * lle2xyz:    0.0,  90.0,   0.0 ->   10.0,   0.0,   0.0
 *
 *               x      y      z ->    lat    lon   elev
 * xyz2lle:   10.0,   0.0,   0.0 ->    0.0,  90.0,   0.0
 * xyz2lle:    0.0,  10.0,   0.0 ->   90.0,   0.0,   0.0
 * xyz2lle:    0.0,   0.0,  10.0 ->    0.0,   0.0,   0.0
 * </programlisting>
 * </example>
 */

#include <glib.h>
#include <math.h>

#include "gis-util.h"

/************
 * GisPoint *
 ************/
/**
 * gis_point_set_lle:
 * @point: the point to modify
 * @lat:   the new latitude
 * @lon:   the new longitude
 * @elev:  the new elevation
 *
 * Set the latitude, longitude, and elevation for a point.
 */
void gis_point_set_lle(GisPoint *point, gdouble lat, gdouble lon, gdouble elev)
{
	point->lat  = lat;
	point->lon  = lon;
	point->elev = elev;
}


/***********
 * GisBBox *
 ***********/
/**
 * gis_bbox_set_bounds:
 * @n: the north edge
 * @s: the south edge
 * @e: the east edge
 * @w: the west edge
 *
 * Set the north, south, east, and west edges of the bounding box
 */
void gis_bbox_set_bounds(GisBBox *bbox,
		gdouble n, gdouble s, gdouble e, gdouble w)
{
	bbox->n = n;
	bbox->s = s;
	bbox->e = e;
	bbox->w = w;
}


/******************
 * Global helpers *
 ******************/
/**
 * lle2xyz:
 * @lat:  the latitude
 * @lon:  the longitude
 * @elev: the elevation
 * @x:    the resulting x coordinate
 * @y:    the resulting y coordinate
 * @z:    the resulting z coordinate
 *
 * Convert a point from latitude, longitude, and elevation to x, y and z
 * coordinates.
 */
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

/**
 * xyz2lle:
 * @x:    the x coordinate
 * @y:    the y coordinate
 * @z:    the z coordinate
 * @lat:  the resulting latitude
 * @lon:  the resulting longitude
 * @elev: the resulting elevation
 *
 * Convert a point from x, y and z coordinates to latitude, longitude, and
 * elevation.
 */
void xyz2lle(gdouble x, gdouble y, gdouble z,
		gdouble *lat, gdouble *lon, gdouble *elev)
{
	gdouble rad = sqrt(x*x + y*y + z*z);
	*lat  = incl2lat(acos(y / rad));
	*lon  = azim2lon(atan2(x,z));
	*elev = rad2elev(rad);
}

/**
 * xyz2ll:
 * @x:    the x coordinate
 * @y:    the y coordinate
 * @z:    the z coordinate
 * @lat:  the resulting latitude
 * @lon:  the resulting longitude
 *
 * Get the latitude and longitude for a x, y, z value.
 */
void xyz2ll(gdouble x, gdouble y, gdouble z,
		gdouble *lat, gdouble *lon)
{
	gdouble rad = sqrt(x*x + y*y + z*z);
	*lat = incl2lat(acos(y / rad));
	*lon = azim2lon(atan2(x,z));
}

/**
 * ll2m:
 * @lon_dist: the distance in degrees of longitude
 * @lat:      the latitude to calculate at
 *
 * Calculate the distance of longitudinal span at a particular latitude. 
 *
 * Returns: the distance in meters
 */
gdouble ll2m(gdouble lon_dist, gdouble lat)
{
	gdouble azim = (-lat+90)/180*M_PI;
	gdouble rad  = sin(azim) * EARTH_R;
	gdouble circ = 2 * M_PI * rad;
	return lon_dist/360 * circ;
}

/**
 * distd:
 * @a: the first point
 * @b: the second point
 *
 * Calculate the distance between two three dimensional points.
 *
 * Returns: the distance between the points
 */
gdouble distd(gdouble *a, gdouble *b)
{
	return sqrt((a[0]-b[0])*(a[0]-b[0]) +
	            (a[1]-b[1])*(a[1]-b[1]) +
	            (a[2]-b[2])*(a[2]-b[2]));
}

/**
 * lon_avg:
 * @a: the first longitude
 * @b: the second longitude
 *
 * Calculate the average longitude between two longitudes. This is smart about
 * which side of the globe the resulting longitude is placed on.
 *
 * Returns: the average
 */
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
