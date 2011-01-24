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

#include <math.h>
#include <glib.h>
#include <gdk/gdkgl.h>
#include <gdk/gdkkeysyms.h>

#include <objects/grits-volume.h>
#include <objects/grits-callback.h>
#include <objects/marching.h>

#include <GL/gl.h>
#include <rsl.h>
#include <tester.h>

/******************
 * iso ball setup *
 ******************/
static double dist  = 0.75;

static gdouble distp(VolPoint *a,
		gdouble bx, gdouble by, gdouble bz)
{
	return 1/((a->c.x-bx)*(a->c.x-bx) +
	          (a->c.y-by)*(a->c.y-by) +
	          (a->c.z-bz)*(a->c.z-bz)) * 0.10;
	//return 1-MIN(1,sqrt((a->c.x-bx)*(a->c.x-bx) +
	//                    (a->c.y-by)*(a->c.y-by) +
	//                    (a->c.z-bz)*(a->c.z-bz)));
}

static VolGrid *load_balls(float dist, int xs, int ys, int zs)
{
	VolGrid *grid = vol_grid_new(xs, ys, zs);
	for (int x = 0; x < xs; x++)
	for (int y = 0; y < ys; y++)
	for (int z = 0; z < zs; z++) {
		VolPoint *point = vol_grid_get(grid, x, y, z);
		point->c.x = ((double)x/(xs-1)*2-1);
		point->c.y = ((double)y/(ys-1)*2-1);
		point->c.z = ((double)z/(zs-1)*2-1);
		point->value =
			distp(point, -dist, 0, 0) +
			distp(point,  dist, 0, 0);
		point->value *= 100;
	}
	return grid;
}

/***************
 * radar setup *
 ***************/
/* Load the radar into a Grits Volume */
static void _cart_to_sphere(VolCoord *out, VolCoord *in)
{
	gdouble angle = in->x;
	gdouble dist  = in->y;
	gdouble tilt  = in->z;
	gdouble lx    = sin(angle);
	gdouble ly    = cos(angle);
	gdouble lz    = sin(tilt);
	out->x = (ly*dist)/20000;
	out->y = (lz*dist)/10000-0.5;
	out->z = (lx*dist)/20000-1.5;
}

static VolGrid *load_radar(gchar *file, gchar *site)
{
	/* Load radar file */
	RSL_read_these_sweeps("all", NULL);
	Radar  *rad = RSL_wsr88d_to_radar(file, site);
	Volume *vol = RSL_get_volume(rad, DZ_INDEX);
	RSL_sort_rays_in_volume(vol);

	/* Count dimensions */
	Sweep *sweep   = vol->sweep[0];
	Ray   *ray     = sweep->ray[0];
	gint nsweeps   = vol->h.nsweeps;
	gint nrays     = sweep->h.nrays/(1/sweep->h.beam_width)+1;
	gint nbins     = ray->h.nbins  /(1000/ray->h.gate_size);
	nbins = MIN(nbins, 100);

	/* Convert to VolGrid */
	VolGrid  *grid = vol_grid_new(nrays, nbins, nsweeps);

	gint rs, bs, val;
	gint si=0, ri=0, bi=0;
	for (si = 0; si < nsweeps; si++) {
		sweep = vol->sweep[si];
		rs    = 1.0/sweep->h.beam_width;
	for (ri = 0; ri < nrays; ri++) {
		/* TODO: missing rays, pick ri based on azimuth */
		ray   = sweep->ray[(ri*rs) % sweep->h.nrays];
		bs    = 1000/ray->h.gate_size;
	for (bi = 0; bi < nbins; bi++) {
		if (bi*bs >= ray->h.nbins)
			break;
		val   = ray->h.f(ray->range[bi*bs]);
		if (val == BADVAL     || val == RFVAL      ||
		    val == APFLAG     || val == NOECHO     ||
		    val == NOTFOUND_H || val == NOTFOUND_V ||
		    val > 80)
			val = 0;
		VolPoint *point = vol_grid_get(grid, ri, bi, si);
		point->value = val;
		point->c.x = deg2rad(ray->h.azimuth);
		point->c.y = bi*bs*ray->h.gate_size + ray->h.range_bin1;
		point->c.z = deg2rad(ray->h.elev);
	} } }

	/* Convert to spherical coords */
	for (si = 0; si < nsweeps; si++)
	for (ri = 0; ri < nrays; ri++)
	for (bi = 0; bi < nbins; bi++) {
		VolPoint *point = vol_grid_get(grid, ri, bi, si);
		if (point->c.y == 0)
			point->value = nan("");
		else
			_cart_to_sphere(&point->c, &point->c);
	}
	return grid;
}

/**********
 * Common *
 **********/
static gboolean key_press(GritsTester *tester, GdkEventKey *event, GritsVolume *volume)
{
	     if (event->keyval == GDK_v) grits_volume_set_level(volume, volume->level-0.5);
	else if (event->keyval == GDK_V) grits_volume_set_level(volume, volume->level+0.5);
	else if (event->keyval == GDK_d) dist  += 0.5;
	else if (event->keyval == GDK_D) dist  -= 0.5;
	return FALSE;
}

/********
 * Main *
 ********/
int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	GtkWidget   *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GritsTester *tester = grits_tester_new();
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(tester));
	gtk_widget_show_all(window);

	/* Grits Volume */
	VolGrid *balls_grid = load_balls(dist, 50, 50, 50);
	GritsVolume *balls = grits_volume_new(balls_grid);
	balls->proj = GRITS_VOLUME_CARTESIAN;
	balls->disp = GRITS_VOLUME_SURFACE;
	balls->color[0] = (0.8)*0xff;
	balls->color[1] = (0.6)*0xff;
	balls->color[2] = (0.2)*0xff;
	balls->color[3] = (1.0)*0xff;
	grits_volume_set_level(balls, 50);
	grits_tester_add(tester, GRITS_OBJECT(balls));
	g_signal_connect(tester, "key-press-event", G_CALLBACK(key_press), balls);

	/* Grits Volume */
	//char *file = "/home/andy/.cache/grits/nexrad/level2/KGWX/KGWX_20101130_0459.raw";
	//char *file = "/home/andy/.cache/grits/nexrad/level2/KTLX/KTLX_19990503_2351.raw";
	//VolGrid *radar_grid = load_radar(file, "KTLX");
	//GritsVolume *radar = grits_volume_new(radar_grid);
	//radar->proj = GRITS_VOLUME_SPHERICAL;
	//radar->disp = GRITS_VOLUME_SURFACE;
	//radar->color[0] = (0.8)*0xff;
	//radar->color[1] = (0.6)*0xff;
	//radar->color[2] = (0.2)*0xff;
	//radar->color[3] = (1.0)*0xff;
	//grits_volume_set_level(radar, 50);
	//grits_tester_add(tester, GRITS_OBJECT(radar));
	//g_signal_connect(tester, "key-press-event", G_CALLBACK(key_press), radar);

	/* Go */
	gtk_main();
	return 0;
}
