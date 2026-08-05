// SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "callbacks.h"
#include "constants.h"
#include "memory-constants.h"
#include "memory.h"


void toggle100StealChance(const ProcessContext processContext, const GameContext gameContext) {
	uint8_t *bytes = gameContext.mask & GUARANTEED_STEAL_TOGGLED
		? (uint8_t[4]){
			STEAL_CHANCE_ORIGINAL_0,
			STEAL_CHANCE_ORIGINAL_1,
			STEAL_CHANCE_ORIGINAL_2,
			STEAL_CHANCE_ORIGINAL_3
		}
		: (uint8_t[4]){
			STEAL_CHANCE_NEW_0,
			STEAL_CHANCE_NEW_1,
			STEAL_CHANCE_NEW_2,
			STEAL_CHANCE_NEW_3
		};
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, STEAL_CHANCE_LOCATION, 4, bytes);
}

void toggleRareStealChance(const ProcessContext processContext, uint8_t rareStealSuccessValue) {
	uint8_t bytes[3] = {RARE_STEAL_CHANCE_NEW_0, RARE_STEAL_CHANCE_NEW_1};
	if (rareStealSuccessValue == RARE_STEAL_CHANCE_ORIGINAL_2) {
		bytes[2] = RARE_STEAL_CHANCE_NEW_2_50_50;
	} else if (rareStealSuccessValue == RARE_STEAL_CHANCE_NEW_2_50_50) {
		bytes[0] = 0x39;
		bytes[2] = 0xc8;
		bytes[2] = NO_OP;
	} else if (rareStealSuccessValue == NO_OP) {
		bytes[2] = RARE_STEAL_CHANCE_NEW_2_NEVER;
	} else {
		bytes[0] = RARE_STEAL_CHANCE_ORIGINAL_0;
		bytes[1] = RARE_STEAL_CHANCE_ORIGINAL_1;
		bytes[2] = RARE_STEAL_CHANCE_ORIGINAL_2;
	}
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, RARE_STEAL_CHANCE_LOCATION, 3, bytes);
}

void toggleAddedSteal(const ProcessContext processContext, const GameContext gameContext) {
	uint8_t *bytes = gameContext.mask & ADDED_STEAL_TOGGLED
		? (uint8_t[2]){
			ADDED_STEAL_ORIGINAL_0,
			ADDED_STEAL_ORIGINAL_1,
		}
		: (uint8_t[2]){NO_OP, NO_OP};
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, ADDED_STEAL_LOCATION, 2, bytes);
}

void toggleRareDropChance(const ProcessContext processContext, uint8_t rareDropSuccessValue) {
	uint8_t bytes[1] = {0};
	if (rareDropSuccessValue == MORE_RARE_DROPS_ORIGINAL) {
		bytes[0] = MORE_RARE_DROPS_NEW_50_50;
	} else if (rareDropSuccessValue == MORE_RARE_DROPS_NEW_50_50) {
		bytes[0] = MORE_RARE_DROPS_NEW_ALWAYS;
	} else if (rareDropSuccessValue == MORE_RARE_DROPS_NEW_ALWAYS) {
		bytes[0] = MORE_RARE_DROPS_NEW_NEVER;
	} else {
		bytes[0] = MORE_RARE_DROPS_ORIGINAL;
	}
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, MORE_RARE_DROPS_LOCATION, 1, bytes);
}

void toggleGuaranteeEquipmentDrop(const ProcessContext processContext, const GameContext gameContext) {
	uint8_t *bytes = gameContext.mask & GUARANTEED_EQUIPMENT_DROP_TOGGLED
		? (uint8_t[1]){ALWAYS_DROP_EQUIPMENT_ORIGINAL}
		: (uint8_t[1]){ALWAYS_DROP_EQUIPMENT_NEW};
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, ALWAYS_DROP_EQUIPMENT_LOCATION, 1, bytes);
}

void togglePerfectSwordplay(const ProcessContext processContext, const GameContext gameContext) {
	uint8_t *bytes = gameContext.mask & PERFECT_TIDUS_OD_TOGGLED
		? (uint8_t[6]){TIDUS_PERFECT_LIMIT_ORIGINAL}
		: (uint8_t[6]){TIDUS_PERFECT_LIMIT_NEW};
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, TIDUS_PERFECT_LIMIT_LOCATION, 6, bytes);
}

void togglePerfectBushido(const ProcessContext processContext, const GameContext gameContext) {
	uint8_t *bytes = gameContext.mask & PERFECT_AURON_OD_TOGGLED
		? (uint8_t[7]){AURON_PERFECT_LIMIT_ORIGINAL}
		: (uint8_t[7]){AURON_PERFECT_LIMIT_NEW};
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, AURON_PERFECT_LIMIT_LOCATION, 7, bytes);
}

void togglePerfectFury(const ProcessContext processContext, const GameContext gameContext) {
	uint8_t *bytes = gameContext.mask & PERFECT_LULU_OD_TOGGLED
		// TODO: put byte length in constant
		? (uint8_t[6]){LULU_PERFECT_LIMIT_ORIGINAL_1}
		: (uint8_t[6]){LULU_PERFECT_LIMIT_NEW_1};
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, LULU_PERFECT_LIMIT_LOCATION_1, 6, bytes);
	bytes = gameContext.mask & PERFECT_LULU_OD_TOGGLED
		? (uint8_t[7]){LULU_PERFECT_LIMIT_ORIGINAL_2}
		: (uint8_t[7]){LULU_PERFECT_LIMIT_NEW_2};
	writeToMemory(processContext.handle, processContext.moduleBaseAddress, LULU_PERFECT_LIMIT_LOCATION_2, 6, bytes);
}

bool handleKeyPress(const ProcessContext processContext, const GameContext gameContext, uint8_t key) {
	switch (key) {
		case '1': {
			toggle100StealChance(processContext, gameContext);
			break;
		}
		case '2': {
			toggleRareStealChance(processContext, gameContext.rareStealSuccessValue);
			break;
		}
		case '3': {
			toggleAddedSteal(processContext, gameContext);
			break;
		}
		case '4': {
			toggleRareDropChance(processContext, gameContext.rareDropSuccessValue);
			break;
		}
		case '5': {
			toggleGuaranteeEquipmentDrop(processContext, gameContext);
			break;
		}
		case '6': {
			togglePerfectSwordplay(processContext, gameContext);
			break;
		}
		case '7': {
			togglePerfectBushido(processContext, gameContext);
			break;
		}
		case '8': {
			togglePerfectFury(processContext, gameContext);
			break;
		}
		default: {
			return false;
		}
	}

	return true;
}
