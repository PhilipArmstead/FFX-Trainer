// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdint.h>


#define bool _Bool
#define false 0
#define true 1

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
} Rectangle16;
