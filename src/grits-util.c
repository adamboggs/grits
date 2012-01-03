/*
 * Copyright (C) 2009-2011 Andy Spencer <andy753421@gmail.com>
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
 * SECTION:grits-util
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
#include <stdio.h>

#include "grits-util.h"

/**************
 * GritsPoint *
 **************/
/**
 * grits_point_set_lle:
 * @point: the point to modify
 * @lat:   the new latitude
 * @lon:   the new longitude
 * @elev:  the new elevation
 *
 * Set the latitude, longitude, and elevation for a point.
 */
void grits_point_set_lle(GritsPoint *point, gdouble lat, gdouble lon, gdouble elev)
{
	point->lat  = lat;
	point->lon  = lon;
	point->elev = elev;
}


/***************
 * GritsBounds *
 ***************/
/**
 * grits_bounds_set_bounds:
 * @n: the north edge
 * @s: the south edge
 * @e: the east edge
 * @w: the west edge
 *
 * Set the north, south, east, and west edges of the bounding box
 */
void grits_bounds_set_bounds(GritsBounds *bounds,
		gdouble n, gdouble s, gdouble e, gdouble w)
{
	bounds->n = n;
	bounds->s = s;
	bounds->e = e;
	bounds->w = w;
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
	gdouble incl = lat2incl(lat);
	gdouble rad  = sin(incl) * EARTH_R;
	gdouble circ = 2 * G_PI * rad;
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

/**
 * crossd3:
 * @a:   the left   point
 * @b:   the center point
 * @c:   the right  point
 * @out: the cross product
 *
 * Calculate the cross product of three points
 */
void crossd3(gdouble *a, gdouble *b, gdouble *c, gdouble *out)
{
	double ba[3], bc[3];

	ba[0] = a[0] - b[0];
	ba[1] = a[1] - b[1];
	ba[2] = a[2] - b[2];

	bc[0] = c[0] - b[0];
	bc[1] = c[1] - b[1];
	bc[2] = c[2] - b[2];

	crossd(ba, bc, out);
}

/**
 * crossd:
 * @a: the first vector
 * @b: the second vector
 * @c: the cross product
 *
 * Calculate the cross product of two vectors
 */
void crossd(gdouble *a, gdouble *b, gdouble *out)
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

/**
 * lengthd:
 * @a: the vector
 *
 * Calculate the length (magnitude) of a vector.
 *
 * Returns: the length
 */
gdouble lengthd(gdouble *a)
{
	return sqrt(a[0] * a[0] +
	            a[1] * a[1] +
	            a[2] * a[2]);
}

/**
 * normd:
 * @a: the first longitude
 *
 * Normalize a vector to a mangitude of 1
 */
void normd(gdouble *a)
{
	gdouble total = lengthd(a);
	a[0] = a[0] / total;
	a[1] = a[1] / total;
	a[2] = a[2] / total;
}

/**
 * parse_points:
 * @string:    String representation of the points
 * @group_sep: Group separator
 * @point_sep: Point separator
 * @coord_sep: Coordinate separator
 * @bounds:    The bounding box of all the points, or NULL
 * @center:    The center of the @bounds, or NULL
 *
 * Parse a string of the form:
 *   string -> group [@group_sep group] ...
 *   group  -> point [@point_sep point] ...
 *   point  -> latitude @coord_sep longitude [@coord_sep elevation]
 *
 * For example
 *   parse_points("30,-80 30,-120 50,-120 50,-80", "\t", " ", ",");
 *
 * Returns: zero-terminated array of groups of points
 */
GritsPoints *parse_points(const gchar *string,
		const gchar *group_sep, const gchar *point_sep, const gchar *coord_sep,
		GritsBounds *bounds, GritsPoint *center)
{
	/* Split and count groups */
	gchar **sgroups = g_strsplit(string, group_sep, -1);
	int     ngroups = g_strv_length(sgroups);

	GritsBounds _bounds = {-90, 90, -180, 180};
	gdouble (**groups)[3] = (gpointer)g_new0(double*, ngroups+1);
	for (int pi = 0; pi < ngroups; pi++) {
		/* Split and count coordinates */
		gchar **scoords = g_strsplit(sgroups[pi], point_sep, -1);
		int     ncoords = g_strv_length(scoords);

		/* Create binary coords */
		gdouble (*coords)[3] = (gpointer)g_new0(gdouble, 3*(ncoords+1));
		for (int ci = 0; ci < ncoords; ci++) {
			gdouble lat, lon;
			sscanf(scoords[ci], "%lf,%lf", &lat, &lon);
			if (bounds || center) {
				if (lat > _bounds.n) _bounds.n = lat;
				if (lat < _bounds.s) _bounds.s = lat;
				if (lon > _bounds.e) _bounds.e = lon;
				if (lon < _bounds.w) _bounds.w = lon;
			}
			lle2xyz(lat, lon, 0,
					&coords[ci][0],
					&coords[ci][1],
					&coords[ci][2]);
		}

		/* Insert coords into line array */
		groups[pi] = coords;
		g_strfreev(scoords);
	}
	g_strfreev(sgroups);

	/* Output */
	if (bounds)
		*bounds = _bounds;
	if (center) {
		center->lat  = (_bounds.n + _bounds.s)/2;
		center->lon  = lon_avg(_bounds.e, _bounds.w);
		center->elev = 0;
	}
	return groups;
}

/**
 * free_points:
 * @points: Array of points allocated by parse_points()
 *
 * Frees all data allocated by parse_points
 */
void free_points(GritsPoints *points)
{
	gdouble (**_points)[3] = points;
	for (int i = 0; _points[i]; i++)
		g_free(_points[i]);
	g_free(_points);
}
