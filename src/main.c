// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdint.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "globals.h"
#include "process.h"
#include "types.h"
#include "ui.h"


static void activate(GtkApplication *app);

int main(int argc, char **argv) {
	#ifdef ARCH_WIN
	uint32_t pid = 0;
	processContext.handle = getProcessFileDescriptor(&pid);
	processContext.pid = pid;
	#else
	processContext.handle = getProcessFileDescriptor();
	#endif

	GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	const int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}

static void activate(GtkApplication *app) {
	ui_initialise(app);

	// Listen for keypresses
	GtkEventController *eventController = gtk_event_controller_key_new();
	g_signal_connect(eventController, "key-pressed", G_CALLBACK(onKeyPress), NULL);
	gtk_widget_add_controller(GTK_WIDGET(windowContext.window), eventController);
}
