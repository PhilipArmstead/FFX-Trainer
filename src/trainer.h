// SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FFX_TRAINER_H
#define FFX_TRAINER_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

enum {
	TRAINER_FEATURE_COUNT = 8,
	TRAINER_STAT_COUNT = 15,
	TRAINER_STAT_CHARS = 11,
	TRAINER_UNKNOWN = 255
};

typedef enum {
	TRAINER_KEY,
	TRAINER_LABEL,
	TRAINER_PERCENT_50,
	TRAINER_PERCENT_100,
	TRAINER_PERCENT_0
} TrainerInput;

typedef enum {
	TRAINER_WAITING,
	TRAINER_CONNECTED,
	TRAINER_UNSUPPORTED,
	TRAINER_ERROR
} TrainerStatus;

typedef struct {
	uint32_t stats[TRAINER_STAT_COUNT];
	uint8_t states[TRAINER_FEATURE_COUNT];
	uint8_t available;
	bool stats_available;
	TrainerStatus status;
	wchar_t detail[256];
} TrainerSnapshot;

void trainer_refresh(TrainerSnapshot *snapshot);
bool trainer_action(unsigned feature, TrainerInput input, TrainerSnapshot *snapshot);
void trainer_close(void);
uint8_t trainer_next_state(unsigned feature, uint8_t state, TrainerInput input);
void trainer_format_stat(wchar_t buffer[TRAINER_STAT_CHARS], uint32_t value);

#endif
