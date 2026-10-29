// SPDX-License-Identifier: GPL-3.0-or-later

#include "trainer_fixture.h"

static void test_inputs_and_patches(void) {
	unsigned feature, state, site;
	for (feature = 0; feature < TRAINER_FEATURE_COUNT; ++feature) {
		reset();
		for (state = 1; state <= s_features[feature].states; ++state) {
			assert(apply_action(feature, TRAINER_KEY));
			assert(s_snapshot.states[feature] == state % s_features[feature].states);
		}
		for (site = 0; site < SITE_COUNT; ++site) {
			assert(memcmp(s_memory + s_sites[site].rva,
				s_sites[site].bytes[0], s_sites[site].size) == 0);
			assert(s_memory[s_sites[site].rva + s_sites[site].size] == 0);
		}
		reset();
		assert(apply_action(feature, TRAINER_LABEL));
		assert(s_snapshot.states[feature] == 1);
		assert(apply_action(feature, TRAINER_LABEL));
		assert(s_snapshot.states[feature] == 0);
	}
	for (feature = 1; feature <= 3; feature += 2) {
		for (state = 0; state < 4; ++state) {
			unsigned percent;
			assert(trainer_next_state(feature, (uint8_t)state, TRAINER_LABEL) == (state ? 0 : 1));
			for (percent = 1; percent <= 3; ++percent) {
				reset();
				if (state) {
					unsigned j;
					for (j = 0; j < state; ++j) {
						assert(apply_action(feature, TRAINER_KEY));
					}
				}
				assert(apply_action(feature, (TrainerInput)(TRAINER_PERCENT_50 + percent - 1)));
				assert(s_snapshot.states[feature] == (state == percent ? 0 : percent));
			}
		}
	}
	assert(trainer_next_state(8, 0, TRAINER_KEY) == TRAINER_UNKNOWN);
	assert(trainer_next_state(0, TRAINER_UNKNOWN, TRAINER_KEY) == TRAINER_UNKNOWN);
	assert(trainer_next_state(0, 0, TRAINER_PERCENT_50) == TRAINER_UNKNOWN);
	assert(trainer_next_state(1, 0, (TrainerInput)99) == TRAINER_UNKNOWN);
	/* Exercise the complete second Fury span, including its seventh byte. */
	reset();
	assert(apply_action(7, TRAINER_KEY));
	assert(s_memory[0x491F76 + 6] == 0x90);
	assert(apply_action(7, TRAINER_KEY));
	assert(s_memory[0x491F76 + 6] == 0x01);
	/* Relocated original operands must round-trip without hard-coded restores. */
	reset();
	s_relocation = 0x100000;
	for (site = 6; site < SITE_COUNT; ++site) {
		site_bytes(&s_sites[site], 0, s_memory + s_sites[site].rva);
	}
	assert(read_snapshot());
	assert(apply_action(7, TRAINER_KEY));
	assert(apply_action(7, TRAINER_KEY));
	assert(read_snapshot());
	assert(s_snapshot.states[7] == 0);
}

static void test_failures(void) {
	reset();
	s_profileReady = false;
	assert(read_snapshot());
	assert(s_snapshot.available == 0 && s_snapshot.stats_available);
	assert(s_snapshot.status == TRAINER_UNSUPPORTED);
	assert(!apply_action(0, TRAINER_KEY) && s_writes == 0);
	reset();
	s_memoryUncertain = true;
	assert(read_snapshot());
	assert(s_snapshot.available == 0 && s_snapshot.status == TRAINER_ERROR);
	assert(!apply_action(0, TRAINER_KEY) && s_writes == 0);
	reset();
	s_memory[s_sites[0].rva] ^= 0xFF;
	assert(read_snapshot());
	assert(s_snapshot.available == 0 && s_snapshot.states[0] == TRAINER_UNKNOWN);
	assert(!apply_action(0, TRAINER_KEY) && s_writes == 0);
	reset();
	site_bytes(&s_sites[8], 1, s_memory + s_sites[8].rva);
	assert(read_snapshot());
	assert(s_snapshot.states[7] == TRAINER_UNKNOWN && s_snapshot.available == 0);
	reset();
	s_memory[s_sites[0].rva] ^= 0xFF;
	assert(!apply_action(0, TRAINER_KEY) && s_writes == 0);
	reset();
	s_failRead = 1;
	assert(!apply_action(0, TRAINER_KEY));
	assert(!s_snapshot.stats_available && s_snapshot.available == 0 && s_writes == 0);
	reset();
	s_failRead = 3;
	s_reads = 0;
	assert(!read_snapshot());
	assert(!s_snapshot.stats_available && s_snapshot.available == 0);
	reset();
	s_failWrites = 1u << 2;
	assert(!apply_action(7, TRAINER_KEY));
	assert(s_snapshot.states[7] == 0 && s_snapshot.available == 0);
	assert(wcsstr(s_snapshot.detail, L"Previous bytes restored"));
	assert(memcmp(s_memory + s_sites[7].rva, s_sites[7].bytes[0], s_sites[7].size) == 0);
	assert(memcmp(s_memory + s_sites[8].rva, s_sites[8].bytes[0], s_sites[8].size) == 0);
	reset();
	s_failWrites = (1u << 2) | (1u << 4);
	assert(!apply_action(7, TRAINER_KEY));
	assert(s_snapshot.states[7] == TRAINER_UNKNOWN && s_snapshot.available == 0);
	assert(wcsstr(s_snapshot.detail, L"ROLLBACK FAILED"));
	reset();
	s_corruptRead = 3;
	assert(!apply_action(7, TRAINER_KEY));
	assert(s_snapshot.states[7] == 0 && s_snapshot.available == 0);
	reset();
	assert(!apply_action(8, TRAINER_KEY));
	assert(!apply_action(0, TRAINER_PERCENT_0) && s_writes == 0);
	reset();
	s_memory[s_sites[0].rva - 1] = 0xCC;
	assert(!apply_action(0, TRAINER_KEY) && s_writes == 0);
	reset();
	s_memory[s_sites[0].rva + s_sites[0].size] = 0xCC;
	assert(!read_snapshot() && s_snapshot.available == 0);
}

static void test_stats(void) {
	static const uint32_t expected[] = {
		0, 200, 100, 201, 101, 202, 102, 204, 104,
		205, 105, 203, 103, 206, 106
	};
	wchar_t text[TRAINER_STAT_CHARS];
	uint32_t max = UINT32_MAX;
	reset();
	assert(memcmp(s_snapshot.stats, expected, sizeof expected) == 0);
	memcpy(s_memory + BATTLES_RVA, &max, sizeof max);
	assert(read_snapshot());
	assert(s_snapshot.stats[0] == max);
	trainer_format_stat(text, max);
	assert(wcscmp(text, L"4294967295") == 0);
	trainer_format_stat(text, 0);
	assert(wcscmp(text, L"0") == 0);
}

int main(void) {
	test_inputs_and_patches();
	test_failures();
	test_stats();
	puts("Trainer logic checks passed.");
	return 0;
}
