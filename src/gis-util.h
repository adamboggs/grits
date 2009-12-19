/*
 * Copyright (C) 2009 Andy Spencer <spenceal@rose-hulman.edu>
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

#ifndef __GIS_UTIL_H__
#define __GIS_UTIL_H__

#define EARTH_R (6371000)
#define EARTH_C (2*M_PI*EARTH_R)
#define NORTH  90
#define SOUTH -90
#define EAST  180
#define WEST -180

/**
 * Terms
 * -----
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
 */

/**
 *             lat    lon   elev ->      x      y      z
 * lle2xyz:    0.0,   0.0,   0.0 ->    0.0,   0.0,  10.0
 * lle2xyz:   90.0,   0.0,   0.0 ->    0.0,  10.0,   0.0
 * lle2xyz:    0.0,  90.0,   0.0 ->   10.0,   0.0,   0.0
 *
 *               x      y      z ->    lat    lon   elev
 * xyz2lle:   10.0,   0.0,   0.0 ->    0.0,  90.0,   0.0
 * xyz2lle:    0.0,  10.0,   0.0 ->   90.0,   0.0,   0.0
 * xyz2lle:    0.0,   0.0,  10.0 ->    0.0,   0.0,   0.0
 */

#define azim2lon(azim) ((azim)*180/M_PI)
#define lon2azim(lon)  ((lon)*M_PI/180)
#define incl2lat(incl) (90-(incl)*180/M_PI)
#define lat2incl(lat)  ((90-(lat))*M_PI/180)
#define rad2elev(rad)  ((rad)-EARTH_R)
#define elev2rad(elev) ((elev)+EARTH_R)

#define deg2rad(deg) (((deg)*M_PI)/180.0)
#define rad2deg(rad) (((rad)*180.0)/M_PI)

#define FOV_DIST   2000.0
#define MPPX(dist) (4*dist/FOV_DIST)

void lle2xyz(gdouble lat, gdouble lon, gdouble elev,
		gdouble *x, gdouble *y, gdouble *z);

void xyz2lle(gdouble x, gdouble y, gdouble z,
		gdouble *lat, gdouble *lon, gdouble *elev);

void xyz2ll(gdouble x, gdouble y, gdouble z,
		gdouble *lat, gdouble *lon);

gdouble ll2m(gdouble lon_dist, gdouble lat);

gdouble distd(gdouble *a, gdouble *b);

gdouble lon_avg(gdouble a, gdouble b);

#endif
