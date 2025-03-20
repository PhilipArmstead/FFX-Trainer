// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later


#pragma once

#include <stdint.h>


static inline uint64_t power(const int base, int exp) {
	uint64_t result = 1;
	while (exp--) result *= base;
	return result;
}

static inline uint64_t hexBytesToInt(const uint8_t *bytes, const char length) {
	uint64_t value = 0;
	for (short i = 0; i < length; ++i) {
		value += (uint64_t)(bytes[i] * power(256, i));
	}
	return value;
}
