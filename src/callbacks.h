// SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "types.h"


void toggle100StealChance(ProcessContext processContext, GameContext gameContext);
void toggleRareStealChance(ProcessContext processContext, uint8_t rareStealSuccessValue);
void toggleAddedSteal(ProcessContext processContext, GameContext gameContext);
void toggleRareDropChance(ProcessContext processContext, uint8_t rareDropSuccessValue);
void toggleGuaranteeEquipmentDrop(ProcessContext processContext, GameContext gameContext);
void togglePerfectSwordplay(ProcessContext processContext, GameContext gameContext);
void togglePerfectBushido(ProcessContext processContext, GameContext gameContext);
void togglePerfectFury(ProcessContext processContext, GameContext gameContext);
bool handleKeyPress(ProcessContext processContext, GameContext gameContext, uint8_t key);
