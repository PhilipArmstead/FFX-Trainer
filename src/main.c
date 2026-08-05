// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdint.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "constants.h"
#include "maths.h"
#include "memory-constants.h"
#include "memory.h"
#include "process.h"
#include "types.h"
#include "callbacks.h"
#include "window.h"
#include "assets/fonts/FreeSans.ttf.h"
// #include "assets/icons/icon.png.h"


static void activate(GtkApplication *app);
static gboolean update(gpointer userData);
G_MODULE_EXPORT void callbackToggle100StealChance(void);
G_MODULE_EXPORT void callbackToggleRareStealChance(void);
G_MODULE_EXPORT void callbackToggleAddedSteal(void);
G_MODULE_EXPORT void callbackSetRareStealChance50(void);
G_MODULE_EXPORT void callbackSetRareStealChance100(void);
G_MODULE_EXPORT void callbackSetRareStealChance0(void);
G_MODULE_EXPORT void callbackToggleRareDropChance(void);
G_MODULE_EXPORT void callbackSetRareDropChance50(void);
G_MODULE_EXPORT void callbackSetRareDropChance100(void);
G_MODULE_EXPORT void callbackSetRareDropChance0(void);
G_MODULE_EXPORT void callbackToggleGuaranteeEquipmentDrop(void);
G_MODULE_EXPORT void callbackTogglePerfectSwordplay(void);
G_MODULE_EXPORT void callbackTogglePerfectBushido(void);
G_MODULE_EXPORT void callbackTogglePerfectFury(void);
static gboolean onKeyPress(
	GtkEventControllerKey *controller,
	guint keyval,
	guint keycode,
	GdkModifierType state,
	gpointer user_data
);
GameContext gameContext = {0};
ProcessContext processContext = {0};
WindowContext windowContext = {0};

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
	char pathToStylesheet[256] = {0};
	GtkCssProvider *cssProvider = gtk_css_provider_new();
	snprintf(pathToStylesheet, sizeof(pathToStylesheet), "%s/app.css", LAYOUTS_DIR);
	gtk_css_provider_load_from_path(cssProvider, pathToStylesheet);
	gtk_style_context_add_provider_for_display(
		gdk_display_get_default(),
		GTK_STYLE_PROVIDER(cssProvider),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
	);
	g_object_unref(cssProvider);

	windowContext = window_create("app", "window:app");
	gtk_window_set_application(GTK_WINDOW(windowContext.window), GTK_APPLICATION(app));

	// Set app version
	#define VERSION_STRING_WIDTH 8
	char version[VERSION_STRING_WIDTH] = {0};
	snprintf(version, VERSION_STRING_WIDTH, "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	GtkWidget *labelVersion = GTK_WIDGET(gtk_builder_get_object(windowContext.builder, "label:version"));
	gtk_label_set_text(GTK_LABEL(labelVersion), version);

	// Listen for keypresses
	GtkEventController *eventController = gtk_event_controller_key_new();
	g_signal_connect(eventController, "key-pressed", G_CALLBACK(onKeyPress), NULL);
	gtk_widget_add_controller(GTK_WIDGET(windowContext.window), eventController);

	// Cache widgets
	GtkBuilder *b = windowContext.builder;
	gameContext.labelBattles = GTK_LABEL(gtk_builder_get_object(b, "label:battles"));
	gameContext.labelTidusKills = GTK_LABEL(gtk_builder_get_object(b, "label:kills:tidus"));
	gameContext.labelTidusVictories = GTK_LABEL(gtk_builder_get_object(b, "label:victories:tidus"));
	gameContext.labelYunaKills = GTK_LABEL(gtk_builder_get_object(b, "label:kills:yuna"));
	gameContext.labelYunaVictories = GTK_LABEL(gtk_builder_get_object(b, "label:victories:yuna"));
	gameContext.labelAuronKills = GTK_LABEL(gtk_builder_get_object(b, "label:kills:auron"));
	gameContext.labelAuronVictories = GTK_LABEL(gtk_builder_get_object(b, "label:victories:auron"));
	gameContext.labelRikkuKills = GTK_LABEL(gtk_builder_get_object(b, "label:kills:kimahri"));
	gameContext.labelRikkuVictories = GTK_LABEL(gtk_builder_get_object(b, "label:victories:kimahri"));
	gameContext.labelLuluKills = GTK_LABEL(gtk_builder_get_object(b, "label:kills:wakka"));
	gameContext.labelLuluVictories = GTK_LABEL(gtk_builder_get_object(b, "label:victories:wakka"));
	gameContext.labelWakkaKills = GTK_LABEL(gtk_builder_get_object(b, "label:kills:lulu"));
	gameContext.labelWakkaVictories = GTK_LABEL(gtk_builder_get_object(b, "label:victories:lulu"));
	gameContext.labelKimahriKills = GTK_LABEL(gtk_builder_get_object(b, "label:kills:rikku"));
	gameContext.labelKimahriVictories = GTK_LABEL(gtk_builder_get_object(b, "label:victories:rikku"));
	gameContext.buttonTogglePerfectSteal = GTK_WIDGET(gtk_builder_get_object(b, "button:toggle:perfect-steal"));
	gameContext.buttonRareStealChance = GTK_WIDGET(gtk_builder_get_object(b, "button:toggle:rare-steal-chance"));
	gameContext.boxRareStealChance = GTK_WIDGET(gtk_builder_get_object(b, "box:rare-steal-chance"));
	gameContext.labelRareStealChance100 = GTK_WIDGET(gtk_builder_get_object(b, "label:rare-steal-chance:100"));
	gameContext.labelRareStealChance50 = GTK_WIDGET(gtk_builder_get_object(b, "label:rare-steal-chance:50"));
	gameContext.labelRareStealChance0 = GTK_WIDGET(gtk_builder_get_object(b, "label:rare-steal-chance:0"));
	gameContext.buttonToggleAddedSteal = GTK_WIDGET(gtk_builder_get_object(b, "button:toggle:added-steal"));
	gameContext.buttonToggleRareDropChance = GTK_WIDGET(gtk_builder_get_object(b, "button:toggle:rare-drop-chance"));
	gameContext.boxRareDropChance = GTK_WIDGET(gtk_builder_get_object(b, "box:rare-drop-chance"));
	gameContext.labelRareDropChance100 = GTK_WIDGET(gtk_builder_get_object(b, "label:rare-drop-chance:100"));
	gameContext.labelRareDropChance50 = GTK_WIDGET(gtk_builder_get_object(b, "label:rare-drop-chance:50"));
	gameContext.labelRareDropChance0 = GTK_WIDGET(gtk_builder_get_object(b, "label:rare-drop-chance:0"));
	gameContext.buttonGuaranteeEquipmentDrop = GTK_WIDGET(
		gtk_builder_get_object(b, "button:toggle:guarantee-equipment-drops")
	);
	gameContext.buttonTogglePerfectSwordplay = GTK_WIDGET(gtk_builder_get_object(b, "button:toggle:perfect-swordplay"));
	gameContext.buttonTogglePerfectBushido = GTK_WIDGET(gtk_builder_get_object(b, "button:toggle:perfect-bushido"));
	gameContext.buttonTogglePerfectFury = GTK_WIDGET(gtk_builder_get_object(b, "button:toggle:perfect-fury"));

	// Periodic callbacks
	g_timeout_add(1000, update, NULL);
	update(NULL);
}

static gboolean update(gpointer userData) {
	#ifdef ARCH_WIN
	bool isGameRunning = processContext.handle != NULL;
	#else
	bool isGameRunning = processContext.handle != -1;
	#endif

	#ifdef ARCH_WIN
	if (isGameRunning && processContext.moduleBaseAddress == 0) {
		processContext.moduleBaseAddress = getModuleBaseAddress(processContext.pid, "FFX.exe");
	}
	#endif

	// These numbers can't both be > 0 and equal.
	// If they are, it means we've failed to read memory correctly
	uint8_t buffer[4];
	readFromMemory(processContext.handle, processContext.moduleBaseAddress, TOTAL_BATTLES_LOCATION, 4, buffer);
	const uint64_t battleCount = hexBytesToInt(buffer, 4);
	readFromMemory(processContext.handle, processContext.moduleBaseAddress, YUNA_VICTORIES_LOCATION, 4, buffer);
	const uint64_t yunaVictories = hexBytesToInt(buffer, 4);

	if (battleCount > 0 && battleCount == yunaVictories) {
		processContext.handle = NULL;
		processContext.moduleBaseAddress = 0;
		return G_SOURCE_CONTINUE;
	}

	{
		char battleCountString[11] = {0};
		snprintf(battleCountString, sizeof(battleCountString), LONG_SPECIFIER, battleCount);
		gtk_label_set_text(gameContext.labelBattles, battleCountString);
	}

	char string[8];
	#define readMemoryInToString(address, label) {										\
		readFromMemory(processContext.handle, processContext.moduleBaseAddress, address, 4, buffer); \
		string[0] = '\0'; \
		snprintf(string, 8, "%d", hexBytesToInt(buffer, 4)); \
		gtk_label_set_text(label, string); \
	}

	readMemoryInToString(TIDUS_KILLS_LOCATION, gameContext.labelTidusKills);
	readMemoryInToString(TIDUS_VICTORIES_LOCATION, gameContext.labelTidusVictories);
	readMemoryInToString(YUNA_KILLS_LOCATION, gameContext.labelYunaKills);
	readMemoryInToString(YUNA_VICTORIES_LOCATION, gameContext.labelYunaVictories);
	readMemoryInToString(AURON_KILLS_LOCATION, gameContext.labelAuronKills);
	readMemoryInToString(AURON_VICTORIES_LOCATION, gameContext.labelAuronVictories);
	readMemoryInToString(KIMAHRI_KILLS_LOCATION, gameContext.labelKimahriKills);
	readMemoryInToString(KIMAHRI_VICTORIES_LOCATION, gameContext.labelKimahriVictories);
	readMemoryInToString(WAKKA_KILLS_LOCATION, gameContext.labelWakkaKills);
	readMemoryInToString(WAKKA_VICTORIES_LOCATION, gameContext.labelWakkaVictories);
	readMemoryInToString(LULU_KILLS_LOCATION, gameContext.labelLuluKills);
	readMemoryInToString(LULU_VICTORIES_LOCATION, gameContext.labelLuluVictories);
	readMemoryInToString(RIKKU_KILLS_LOCATION, gameContext.labelRikkuKills);
	readMemoryInToString(RIKKU_VICTORIES_LOCATION, gameContext.labelRikkuVictories);

	readFromMemory(processContext.handle, processContext.moduleBaseAddress, STEAL_CHANCE_LOCATION, 4, buffer);
	if (
		buffer[0] != STEAL_CHANCE_ORIGINAL_0 ||
		buffer[1] != STEAL_CHANCE_ORIGINAL_1 ||
		buffer[2] != STEAL_CHANCE_ORIGINAL_2 ||
		buffer[3] != STEAL_CHANCE_ORIGINAL_3
	) {
		gameContext.mask |= GUARANTEED_STEAL_TOGGLED;
	} else {
		gameContext.mask &= ~GUARANTEED_STEAL_TOGGLED;
	}
	readFromMemory(processContext.handle, processContext.moduleBaseAddress, RARE_STEAL_CHANCE_LOCATION + 2, 1, buffer);
	gameContext.rareStealSuccessValue = buffer[0];
	readFromMemory(processContext.handle, processContext.moduleBaseAddress, ADDED_STEAL_LOCATION, 3, buffer);
	if (buffer[0] != ADDED_STEAL_ORIGINAL_0 || buffer[1] != ADDED_STEAL_ORIGINAL_1) {
		gameContext.mask |= ADDED_STEAL_TOGGLED;
	} else {
		gameContext.mask &= ~ADDED_STEAL_TOGGLED;
	}
	readFromMemory(processContext.handle, processContext.moduleBaseAddress, MORE_RARE_DROPS_LOCATION, 1, buffer);
	gameContext.rareDropSuccessValue = buffer[0];

	readFromMemory(processContext.handle, processContext.moduleBaseAddress, TIDUS_PERFECT_LIMIT_LOCATION + 5, 1, buffer);
	if (buffer[0] == NO_OP) {
		gameContext.mask |= PERFECT_TIDUS_OD_TOGGLED;
	} else {
		gameContext.mask &= ~PERFECT_TIDUS_OD_TOGGLED;
	}

	readFromMemory(processContext.handle, processContext.moduleBaseAddress, LULU_PERFECT_LIMIT_LOCATION_2 + 5, 1, buffer);
	if (buffer[0] == NO_OP) {
		gameContext.mask |= PERFECT_LULU_OD_TOGGLED;
	} else {
		gameContext.mask &= ~PERFECT_LULU_OD_TOGGLED;
	}
	readFromMemory(processContext.handle, processContext.moduleBaseAddress, AURON_PERFECT_LIMIT_LOCATION + 5, 1, buffer);
	if (buffer[0] == NO_OP) {
		gameContext.mask |= PERFECT_AURON_OD_TOGGLED;
	} else {
		gameContext.mask &= ~PERFECT_AURON_OD_TOGGLED;
	}
	readFromMemory(processContext.handle, processContext.moduleBaseAddress, ALWAYS_DROP_EQUIPMENT_LOCATION, 1, buffer);
	if (buffer[0] != ALWAYS_DROP_EQUIPMENT_ORIGINAL) {
		gameContext.mask |= GUARANTEED_EQUIPMENT_DROP_TOGGLED;
	} else {
		gameContext.mask &= ~GUARANTEED_EQUIPMENT_DROP_TOGGLED;
	}

	if (gameContext.mask & GUARANTEED_STEAL_TOGGLED) {
		gtk_widget_add_css_class(gameContext.buttonTogglePerfectSteal, "hack-active");
	} else {
		gtk_widget_remove_css_class(gameContext.buttonTogglePerfectSteal, "hack-active");
	}

	if (gameContext.rareStealSuccessValue != RARE_STEAL_CHANCE_ORIGINAL_2) {
		gtk_widget_add_css_class(gameContext.buttonRareStealChance, "hack-active");
		gtk_widget_set_visible(gameContext.boxRareStealChance, true);

		if (gameContext.rareStealSuccessValue == RARE_STEAL_CHANCE_NEW_2_50_50) {
			gtk_widget_add_css_class(gameContext.labelRareStealChance50, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareStealChance100, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareStealChance0, "hack-active");
		} else if (gameContext.rareStealSuccessValue == RARE_STEAL_CHANCE_NEW_2_NEVER) {
			gtk_widget_add_css_class(gameContext.labelRareStealChance0, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareStealChance100, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareStealChance50, "hack-active");
		} else {
			gtk_widget_add_css_class(gameContext.labelRareStealChance100, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareStealChance0, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareStealChance50, "hack-active");
		}
	} else {
		gtk_widget_set_visible(gameContext.boxRareStealChance, false);
		gtk_widget_remove_css_class(gameContext.buttonRareStealChance, "hack-active");
	}

	if (gameContext.mask & ADDED_STEAL_TOGGLED) {
		gtk_widget_add_css_class(gameContext.buttonToggleAddedSteal, "hack-active");
	} else {
		gtk_widget_remove_css_class(gameContext.buttonToggleAddedSteal, "hack-active");
	}

	if (gameContext.rareDropSuccessValue != MORE_RARE_DROPS_ORIGINAL) {
		gtk_widget_add_css_class(gameContext.buttonToggleRareDropChance, "hack-active");
		gtk_widget_set_visible(gameContext.boxRareDropChance, true);

		if (gameContext.rareDropSuccessValue == MORE_RARE_DROPS_NEW_50_50) {
			gtk_widget_add_css_class(gameContext.labelRareDropChance50, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareDropChance100, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareDropChance0, "hack-active");
		} else if (gameContext.rareDropSuccessValue == MORE_RARE_DROPS_NEW_NEVER) {
			gtk_widget_add_css_class(gameContext.labelRareDropChance0, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareDropChance100, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareDropChance50, "hack-active");
		} else {
			gtk_widget_add_css_class(gameContext.labelRareDropChance100, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareDropChance0, "hack-active");
			gtk_widget_remove_css_class(gameContext.labelRareDropChance50, "hack-active");
		}
	} else {
		gtk_widget_set_visible(gameContext.boxRareDropChance, false);
		gtk_widget_remove_css_class(gameContext.buttonToggleRareDropChance, "hack-active");
	}

	if (gameContext.mask & GUARANTEED_EQUIPMENT_DROP_TOGGLED) {
		gtk_widget_add_css_class(gameContext.buttonGuaranteeEquipmentDrop, "hack-active");
	} else {
		gtk_widget_remove_css_class(gameContext.buttonGuaranteeEquipmentDrop, "hack-active");
	}

	if (gameContext.mask & PERFECT_TIDUS_OD_TOGGLED) {
		gtk_widget_add_css_class(gameContext.buttonTogglePerfectSwordplay, "hack-active");
	} else {
		gtk_widget_remove_css_class(gameContext.buttonTogglePerfectSwordplay, "hack-active");
	}

	if (gameContext.mask & PERFECT_AURON_OD_TOGGLED) {
		gtk_widget_add_css_class(gameContext.buttonTogglePerfectBushido, "hack-active");
	} else {
		gtk_widget_remove_css_class(gameContext.buttonTogglePerfectBushido, "hack-active");
	}

	if (gameContext.mask & PERFECT_LULU_OD_TOGGLED) {
		gtk_widget_add_css_class(gameContext.buttonTogglePerfectFury, "hack-active");
	} else {
		gtk_widget_remove_css_class(gameContext.buttonTogglePerfectFury, "hack-active");
	}

	return G_SOURCE_CONTINUE;
}

G_MODULE_EXPORT void callbackToggle100StealChance(void) {
	toggle100StealChance(processContext, gameContext);
	update(NULL);
}

G_MODULE_EXPORT void callbackToggleRareStealChance(void) {
	toggleRareStealChance(processContext, gameContext.rareStealSuccessValue);
	update(NULL);
}

G_MODULE_EXPORT void callbackSetRareStealChance50(void) {
	toggleRareStealChance(processContext, RARE_STEAL_CHANCE_ORIGINAL_2);
	update(NULL);
}

G_MODULE_EXPORT void callbackSetRareStealChance100(void) {
	toggleRareStealChance(processContext, RARE_STEAL_CHANCE_NEW_2_50_50);
	update(NULL);
}

G_MODULE_EXPORT void callbackSetRareStealChance0(void) {
	toggleRareStealChance(processContext, NO_OP);
	update(NULL);
}

G_MODULE_EXPORT void callbackToggleRareDropChance(void) {
	toggleRareDropChance(processContext, gameContext.rareDropSuccessValue);
	update(NULL);
}

G_MODULE_EXPORT void callbackSetRareDropChance50(void) {
	toggleRareDropChance(processContext, MORE_RARE_DROPS_ORIGINAL);
	update(NULL);
}

G_MODULE_EXPORT void callbackSetRareDropChance100(void) {
	toggleRareDropChance(processContext, MORE_RARE_DROPS_NEW_50_50);
	update(NULL);
}

G_MODULE_EXPORT void callbackSetRareDropChance0(void) {
	toggleRareDropChance(processContext, MORE_RARE_DROPS_NEW_ALWAYS);
	update(NULL);
}

G_MODULE_EXPORT void callbackToggleAddedSteal(void) {
	toggleAddedSteal(processContext, gameContext);
	update(NULL);
}

G_MODULE_EXPORT void callbackToggleGuaranteeEquipmentDrop(void) {
	toggleGuaranteeEquipmentDrop(processContext, gameContext);
	update(NULL);
}

G_MODULE_EXPORT void callbackTogglePerfectSwordplay(void) {
	togglePerfectSwordplay(processContext, gameContext);
	update(NULL);
}

G_MODULE_EXPORT void callbackTogglePerfectBushido(void) {
	togglePerfectBushido(processContext, gameContext);
	update(NULL);
}

G_MODULE_EXPORT void callbackTogglePerfectFury(void) {
	togglePerfectFury(processContext, gameContext);
	update(NULL);
}

static gboolean onKeyPress(
	GtkEventControllerKey *controller,
	guint keyval,
	guint keycode,
	const GdkModifierType state,
	gpointer user_data
) {
	const bool wasHandled = handleKeyPress(processContext, gameContext, keycode);
	if (wasHandled) {
		update(NULL);
	}

	return wasHandled;
}
