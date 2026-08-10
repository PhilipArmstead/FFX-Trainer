// SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "types.h"


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
gboolean onKeyPress(
	GtkEventControllerKey *controller,
	guint keyval,
	guint keycode,
	GdkModifierType state,
	gpointer user_data
);
