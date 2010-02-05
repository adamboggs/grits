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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gis-util.h"
#include "data/gis-wms.h"
#include "objects/gis-tile.h"

struct CacheState {
	GtkWidget *image;
	GtkWidget *status;
	GtkWidget *progress;
};


void chunk_callback(gsize cur, gsize total, gpointer _state)
{
	struct CacheState *state = _state;
	g_message("chunk_callback: %d/%d", cur, total);

	if (state->progress == NULL) {
		state->progress = gtk_progress_bar_new();
		gtk_box_pack_end(GTK_BOX(state->status), state->progress, FALSE, FALSE, 0);
		gtk_widget_show(state->progress);
	}

	if (cur == total)
		gtk_widget_destroy(state->progress);
	else
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(state->progress), (gdouble)cur/total);
}

gpointer do_bmng_cache(gpointer _image)
{
	GtkImage *image = _image;
	g_message("Creating bmng tile");
	GisTile *tile = gis_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	tile->children[0][1] = gis_tile_new(tile, NORTH, 0, 0, WEST);
	tile = tile->children[0][1];

	g_message("Fetching bmng image");
	GisWms *bmng_wms = gis_wms_new(
		"http://www.nasa.network.com/wms", "bmng200406", "image/jpeg",
		"bmng_test/", "jpg", 512, 256);
	const char *path = gis_wms_make_local(bmng_wms, tile);

	g_message("Loading bmng image: [%s]", path);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	gdk_threads_enter();
	gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
	gdk_threads_leave();

	g_message("Cleaning bmng up");
	gis_wms_free(bmng_wms);
	gis_tile_free(tile, NULL, NULL);
	return NULL;
}

gpointer do_osm_cache(gpointer _image)
{
	GtkImage *image = _image;
	g_message("Creating osm tile");
	GisTile *tile = gis_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	tile->children[0][1] = gis_tile_new(tile, NORTH, 0, 0, WEST);
	tile = tile->children[0][1];

	g_message("Fetching osm image");
	GisWms *osm_wms = gis_wms_new(
		"http://labs.metacarta.com/wms/vmap0", "basic", "image/png",
		"osm_test/", "png", 512, 256);
	const char *path = gis_wms_make_local(osm_wms, tile);

	g_message("Loading osm image: [%s]", path);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	gdk_threads_enter();
	gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
	gdk_threads_leave();

	g_message("Cleaning osm up");
	gis_wms_free(osm_wms);
	gis_tile_free(tile, NULL, NULL);
	return NULL;
}

gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (event->keyval == GDK_q)
		gtk_main_quit();
	return TRUE;
}

int main(int argc, char **argv)
{
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);

	GtkWidget *win        = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *vbox1      = gtk_vbox_new(FALSE, 0);
	GtkWidget *vbox2      = gtk_vbox_new(FALSE, 0);
	GtkWidget *status     = gtk_statusbar_new();
	GtkWidget *scroll     = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget *bmng_image = gtk_image_new();
	GtkWidget *srtm_image = gtk_image_new();
	GtkWidget *osm_image  = gtk_image_new();
	gtk_container_add(GTK_CONTAINER(win), vbox1);
	gtk_box_pack_start(GTK_BOX(vbox1), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), vbox2);
	gtk_box_pack_start(GTK_BOX(vbox2), bmng_image, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), srtm_image, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), osm_image,  TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), status, FALSE, FALSE, 0);
	g_signal_connect(win, "key-press-event", G_CALLBACK(key_press_cb), NULL);
	g_signal_connect(win, "destroy", gtk_main_quit, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	g_thread_create(do_bmng_cache, bmng_image, FALSE, NULL);
	g_thread_create(do_osm_cache,  osm_image,  FALSE, NULL);

	gtk_widget_show_all(win);
	gtk_main();

	return 0;
}
