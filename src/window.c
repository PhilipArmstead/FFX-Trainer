// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"

#include <gtk/gtk.h>


WindowContext window_create(const char *layoutName, const char *windowName) {
	char pathToAppLayout[256] = {0};
	snprintf(pathToAppLayout, sizeof(pathToAppLayout), "%s/%s.ui", LAYOUTS_DIR, layoutName);

	WindowContext context = {};
	context.builder = gtk_builder_new_from_file(pathToAppLayout);
	context.window = GTK_WIDGET(gtk_builder_get_object(context.builder, windowName));
	gtk_window_present(GTK_WINDOW(context.window));
	return context;
}