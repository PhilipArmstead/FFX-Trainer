// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later


#pragma once

#define VERSION_MAJOR 0
#define VERSION_MINOR 9
#define VERSION_PATCH 0

#define LINE_HEIGHT 26
#define PADDING 24
#define STATS_RIGHT 75

const char *perfectStealString = "1) Toggle 100% steal chance";
const char *rareStealString = "2) Toggle rare steal chance";
const char *addedStealString = "3) Toggle added steal";
const char *rareDropString = "4) Toggle rare drop chance";
const char *alwaysDropEquipmentString = "5) Toggle always drop equipment";
const char *perfectSwordplayString = "6) Toggle perfect Swordplay";
const char *perfectBushidoString = "7) Toggle perfect Bushido";
const char *perfectFuryString = "8) Toggle perfect Fury";

const char *battlesString = "Battles:";
const char *tidusKillsString = "Tidus kills:";
const char *tidusVictoriesString = "Tidus victories:";
const char *yunaKillsString = "Yuna kills:";
const char *yunaVictoriesString = "Yuna victories:";
const char *auronKillsString = "Auron kills:";
const char *auronVictoriesString = "Auron victories:";
const char *wakkaKillsString = "Wakka kills:";
const char *wakkaVictoriesString = "Wakka victories:";
const char *luluKillsString = "Lulu kills:";
const char *luluVictoriesString = "Lulu victories:";
const char *kimahriKillsString = "Kimahri kills:";
const char *kimahriVictoriesString = "Kimahri victories:";
const char *rikkuKillsString = "Rikku kills:";
const char *rikkuVictoriesString = "Rikku victories:";

#define PERFECT_TIDUS_OD_TOGGLED (1 << 0)
#define PERFECT_LULU_OD_TOGGLED (1 << 1)
#define PERFECT_AURON_OD_TOGGLED (1 << 2)
#define GUARANTEED_STEAL_TOGGLED (1 << 3)
#define ADDED_STEAL_TOGGLED (1 << 4)
#define GUARANTEED_EQUIPMENT_DROP_TOGGLED (1 << 5)
