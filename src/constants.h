// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later


#pragma once

#define LINE_HEIGHT 26
#define PADDING_LEFT 24

const char *perfectStealString = "1) Toggle 100% steal chance";
const char *rareStealString = "2) Toggle rare steal chance";
const char *addedStealString = "3) Toggle added steal";
const char *rareDropString = "4) Toggle rare drop chance";
const char *alwaysDropEquipmentString = "5) Toggle always drop equipment";
const char *perfectSwordplayString = "6) Toggle perfect Swordplay";
const char *perfectBushidoString = "7) Toggle perfect Bushido";
const char *perfectFuryString = "8) Toggle perfect Fury";

#define PERFECT_TIDUS_OD_TOGGLED (1 << 0)
#define PERFECT_LULU_OD_TOGGLED (1 << 1)
#define PERFECT_AURON_OD_TOGGLED (1 << 2)
#define GUARANTEED_STEAL_TOGGLED (1 << 3)
#define ADDED_STEAL_TOGGLED (1 << 4)
#define GUARANTEED_EQUIPMENT_DROP_TOGGLED (1 << 5)
