// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtk/gtk.h>


#define bool _Bool
#define false 0
#define true 1

#define GAME_STATUS_STRING_BUFFER_SIZE 32

typedef struct {
	char gameVersion[GAME_STATUS_STRING_BUFFER_SIZE];
	GtkLabel *labelBattles;
	GtkLabel *labelTidusKills;
	GtkLabel *labelTidusVictories;
	GtkLabel *labelYunaKills;
	GtkLabel *labelYunaVictories;
	GtkLabel *labelAuronKills;
	GtkLabel *labelAuronVictories;
	GtkLabel *labelRikkuKills;
	GtkLabel *labelRikkuVictories;
	GtkLabel *labelLuluKills;
	GtkLabel *labelLuluVictories;
	GtkLabel *labelWakkaKills;
	GtkLabel *labelWakkaVictories;
	GtkLabel *labelKimahriKills;
	GtkLabel *labelKimahriVictories;
	GtkWidget *buttonTogglePerfectSteal;
	GtkWidget *boxRareStealChance;
	GtkWidget *buttonRareStealChance;
	GtkWidget *labelRareStealChance100;
	GtkWidget *labelRareStealChance50;
	GtkWidget *labelRareStealChance0;
	GtkWidget *buttonToggleAddedSteal;
	GtkWidget *buttonToggleRareDropChance;
	GtkWidget *boxRareDropChance;
	GtkWidget *labelRareDropChance100;
	GtkWidget *labelRareDropChance50;
	GtkWidget *labelRareDropChance0;
	GtkWidget *buttonGuaranteeEquipmentDrop;
	GtkWidget *buttonTogglePerfectSwordplay;
	GtkWidget *buttonTogglePerfectBushido;
	GtkWidget *buttonTogglePerfectFury;
	/**
	 * Bit 1 = is perfect Tidus OD toggled?
	 * Bit 2 = is perfect Lulu OD toggled?
	 * Bit 3 = is perfect Auron OD toggled?
	 * Bit 4 = is steal success rate toggled?
	 * Bit 5 = is added steal toggled?
	 * Bit 6 = is guaranteed equipment drops toggled?
	*/
	uint8_t mask;
	uint8_t rareDropSuccessValue;
	uint8_t rareStealSuccessValue;
} GameContext;

typedef struct {
	GtkBuilder *builder;
	GtkWidget *window;
} WindowContext;

// Process
typedef struct {
	void *handle; // Platform-specific process handle
	uintptr_t moduleBaseAddress;
	uint32_t pid;
} ProcessContext;
