// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FFX_TRAINER_FIXTURE_H
#define FFX_TRAINER_FIXTURE_H

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#define TRAINER_TEST
#include "../src/trainer.c"

static uint8_t s_memory[CHARACTER_BLOCK + CHARACTER_BYTES];
static unsigned s_reads, s_writes;
static unsigned s_failRead, s_failWrites, s_corruptRead;

static bool read_memory(uint32_t rva, void *buffer, size_t size) {
	++s_reads;
	if (s_reads == s_failRead || rva > sizeof s_memory || size > sizeof s_memory - rva) {
		report(TRAINER_ERROR, L"Injected read failure.");
		return false;
	}
	memcpy(buffer, s_memory + rva, size);
	if (s_reads == s_corruptRead && size) {
		((uint8_t *)buffer)[0] ^= 0xFF;
	}
	return true;
}

static bool write_memory(uint32_t rva, const void *buffer, size_t size) {
	++s_writes;
	assert(rva <= sizeof s_memory && size <= sizeof s_memory - rva);
	if (s_writes < 32 && (s_failWrites & (1u << s_writes))) {
		memcpy(s_memory + rva, buffer, 1);
		report(TRAINER_ERROR, L"Injected partial write failure.");
		return false;
	}
	memcpy(s_memory + rva, buffer, size);
	return true;
}

static void reset(void) {
	unsigned site, slot;
	memset(&s_snapshot, 0, sizeof s_snapshot);
	clear_values();
	memset(s_memory, 0, sizeof s_memory);
	s_reads = s_writes = s_failRead = s_failWrites = s_corruptRead = 0;
	s_relocation = 0;
	s_memoryUncertain = false;
	s_profileReady = true;
	for (site = 0; site < SITE_COUNT; ++site) {
		site_bytes(&s_sites[site], 0, s_memory + s_sites[site].rva);
		memcpy(s_context[site], s_memory + s_sites[site].rva - CONTEXT_MARGIN,
			(size_t)s_sites[site].size + CONTEXT_MARGIN * 2);
	}
	for (slot = 0; slot < 7; ++slot) {
		uint32_t victories = 100 + slot, kills = 200 + slot;
		memcpy(s_memory + CHARACTER_BLOCK + slot * CHARACTER_STRIDE, &victories, 4);
		memcpy(s_memory + CHARACTER_BLOCK + slot * CHARACTER_STRIDE + 4, &kills, 4);
	}
	assert(read_snapshot());
	assert(s_snapshot.available == UINT8_MAX);
	s_reads = 0;
}

#endif
