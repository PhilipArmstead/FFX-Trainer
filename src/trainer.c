// SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trainer.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef TRAINER_TEST
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#ifndef TRAINER_PROCESS_NAME
#define TRAINER_PROCESS_NAME L"FFX.exe"
#endif
#endif

enum {
	MAX_PATCH_BYTES = 7,
	CONTEXT_MARGIN = 8,
	MAX_CONTEXT_BYTES = MAX_PATCH_BYTES + CONTEXT_MARGIN * 2,
	SITE_COUNT = 9,
	CHARACTER_BLOCK = 0xD320AC,
	CHARACTER_STRIDE = 0x94,
	CHARACTER_BYTES = CHARACTER_STRIDE * 6 + 8,
	BATTLES_RVA = 0xD307A4
};

typedef struct {
	uint32_t rva;
	uint8_t size;
	int8_t relocation;
	uint8_t bytes[4][MAX_PATCH_BYTES];
} PatchSite;

typedef struct {
	uint8_t first;
	uint8_t count;
	uint8_t states;
} Feature;

/* Windows Steam RVAs; validate against the executable before enabling writes. */
static const PatchSite s_sites[SITE_COUNT] = {
	{0x38B830, 4, -1, {{0x0F, 0xB6, 0x5E, 0x0A}, {0x66, 0xBB, 0xFF, 0x00}}},
	{0x38B905, 3, -1, {
		{0x3B, 0x45, 0x10}, {0x83, 0xF8, 0x7F},
		{0x39, 0xC8, 0x90}, {0x83, 0xF8, 0x00}
	}},
	{0x38956B, 2, -1, {{0x74, 0x1C}, {0x90, 0x90}}},
	{0x3992D1, 1, -1, {{0x20}, {0x7F}, {0xFF}, {0x00}}},
	{0x399394, 1, -1, {{0x7C}, {0xEB}}},
	{0x4923EE, 6, -1, {
		{0x0F, 0x8D, 0x71, 0xFF, 0xFF, 0xFF},
		{0xE9, 0x67, 0xFF, 0xFF, 0xFF, 0x90}
	}},
	{0x490F11, 7, 3, {
		{0x66, 0xFF, 0x05, 0xB4, 0xD6, 0x33, 0x01},
		{0xE9, 0xB0, 0x00, 0x00, 0x00, 0x90, 0x90}
	}},
	{0x491C18, 6, 2, {
		{0xD8, 0x1D, 0x50, 0xF7, 0x33, 0x01},
		{0xE9, 0x13, 0x03, 0x00, 0x00, 0x90}
	}},
	{0x491F76, 7, 3, {
		{0x0F, 0xB6, 0x05, 0xDF, 0xD6, 0x33, 0x01},
		{0xB8, 0x0C, 0x00, 0x00, 0x00, 0x90, 0x90}
	}}
};

static const Feature s_features[TRAINER_FEATURE_COUNT] = {
	{0, 1, 2}, {1, 1, 4}, {2, 1, 2}, {3, 1, 4},
	{4, 1, 2}, {5, 1, 2}, {6, 1, 2}, {7, 2, 2}
};

/* Display order: Tidus, Yuna, Auron, Wakka, Lulu, Kimahri, Rikku. */
static const uint8_t s_characterSlots[7] = {0, 1, 2, 4, 5, 3, 6};
static TrainerSnapshot s_snapshot;
static uint32_t s_relocation;
static bool s_memoryUncertain;
static bool s_profileReady;
static uint8_t s_context[SITE_COUNT][MAX_CONTEXT_BYTES];

static bool read_memory(uint32_t rva, void *buffer, size_t size);
static bool write_memory(uint32_t rva, const void *buffer, size_t size);

static void clear_values(void) {
	memset(s_snapshot.stats, 0, sizeof s_snapshot.stats);
	memset(s_snapshot.states, TRAINER_UNKNOWN, sizeof s_snapshot.states);
	s_snapshot.available = 0;
	s_snapshot.stats_available = false;
}

static void report(TrainerStatus status, const wchar_t *message) {
	s_snapshot.status = status;
	swprintf(s_snapshot.detail, sizeof s_snapshot.detail / sizeof *s_snapshot.detail, L"%ls", message);
}

uint8_t trainer_next_state(unsigned feature, uint8_t state, TrainerInput input) {
	if (feature >= TRAINER_FEATURE_COUNT || state >= s_features[feature].states) {
		return TRAINER_UNKNOWN;
	}
	if (input == TRAINER_KEY) {
		return (uint8_t)((state + 1u) % s_features[feature].states);
	}
	if (input == TRAINER_LABEL) {
		return state ? 0 : 1;
	}
	if (s_features[feature].states == 4 &&
		input >= TRAINER_PERCENT_50 && input <= TRAINER_PERCENT_0) {
		uint8_t target = (uint8_t)(input - TRAINER_PERCENT_50 + 1);
		return state == target ? 0 : target;
	}
	return TRAINER_UNKNOWN;
}

void trainer_format_stat(wchar_t buffer[TRAINER_STAT_CHARS], uint32_t value) {
	swprintf(buffer, TRAINER_STAT_CHARS, L"%lu", (unsigned long)value);
}

static void site_bytes(const PatchSite *site, unsigned state, uint8_t *buffer) {
	memcpy(buffer, site->bytes[state], site->size);
	if (state == 0 && site->relocation >= 0) {
		uint32_t address;
		memcpy(&address, buffer + site->relocation, sizeof address);
		address += s_relocation;
		memcpy(buffer + site->relocation, &address, sizeof address);
	}
}

static uint8_t decode_feature(unsigned feature, uint8_t bytes[2][MAX_PATCH_BYTES]) {
	const Feature *def = &s_features[feature];
	unsigned state, i;
	for (state = 0; state < def->states; ++state) {
		for (i = 0; i < def->count; ++i) {
			uint8_t expected[MAX_PATCH_BYTES];
			const PatchSite *site = &s_sites[def->first + i];
			site_bytes(site, state, expected);
			if (memcmp(expected, bytes[i], site->size) != 0) {
				break;
			}
		}
		if (i == def->count) {
			return (uint8_t)state;
		}
	}
	return TRAINER_UNKNOWN;
}

static bool read_feature(unsigned feature, uint8_t bytes[2][MAX_PATCH_BYTES]) {
	const Feature *def = &s_features[feature];
	unsigned i;
	for (i = 0; i < def->count; ++i) {
		unsigned index = def->first + i;
		const PatchSite *site = &s_sites[index];
		uint8_t context[MAX_CONTEXT_BYTES];
		if (!read_memory(site->rva - CONTEXT_MARGIN, context,
			(size_t)site->size + CONTEXT_MARGIN * 2)) {
			return false;
		}
		if (memcmp(context, s_context[index], CONTEXT_MARGIN) != 0 ||
			memcmp(context + CONTEXT_MARGIN + site->size,
				s_context[index] + CONTEXT_MARGIN + site->size, CONTEXT_MARGIN) != 0) {
			report(TRAINER_UNSUPPORTED, L"Instructions surrounding a patch changed. No writes are allowed.");
			return false;
		}
		memcpy(bytes[i], context + CONTEXT_MARGIN, site->size);
	}
	return true;
}

static void decode_stats(uint32_t battles, const uint8_t *characters, uint32_t *stats) {
	unsigned i;
	stats[0] = battles;
	for (i = 0; i < 7; ++i) {
		const uint8_t *pair = characters + s_characterSlots[i] * CHARACTER_STRIDE;
		memcpy(&stats[1 + i * 2], pair + 4, sizeof(uint32_t));
		memcpy(&stats[2 + i * 2], pair, sizeof(uint32_t));
	}
}

static bool read_snapshot(void) {
	uint32_t battles, stats[TRAINER_STAT_COUNT];
	uint8_t characters[CHARACTER_BYTES];
	uint8_t states[TRAINER_FEATURE_COUNT];
	unsigned feature;
	bool recognized = true;

	if (!read_memory(BATTLES_RVA, &battles, sizeof battles) ||
		!read_memory(CHARACTER_BLOCK, characters, sizeof characters)) {
		clear_values();
		return false;
	}
	decode_stats(battles, characters, stats);
	for (feature = 0; feature < TRAINER_FEATURE_COUNT; ++feature) {
		uint8_t bytes[2][MAX_PATCH_BYTES] = {{0}};
		if (!read_feature(feature, bytes)) {
			clear_values();
			return false;
		}
		states[feature] = decode_feature(feature, bytes);
		if (states[feature] == TRAINER_UNKNOWN) {
			recognized = false;
		}
	}
	memcpy(s_snapshot.stats, stats, sizeof stats);
	memcpy(s_snapshot.states, states, sizeof states);
	s_snapshot.stats_available = true;
	s_snapshot.available = recognized && s_profileReady && !s_memoryUncertain ? UINT8_MAX : 0;
	if (s_memoryUncertain) {
		report(TRAINER_ERROR, L"A previous write could not restore code protection. Restart the game; all writes are disabled.");
	} else if (!recognized) {
		report(TRAINER_UNSUPPORTED, L"Unrecognized or mixed patch bytes. All writes are disabled.");
	} else if (!s_profileReady) {
		report(TRAINER_UNSUPPORTED, L"The game executable has not passed memory-profile validation.");
	} else {
		report(TRAINER_CONNECTED, L"Connected to FFX.exe.");
	}
	return true;
}

static bool apply_action(unsigned feature, TrainerInput input) {
	uint8_t before[2][MAX_PATCH_BYTES] = {{0}};
	uint8_t after[2][MAX_PATCH_BYTES] = {{0}};
	const Feature *def;
	uint8_t state, target;
	unsigned i, attempted = 0;
	bool restored = true;
	wchar_t failure[256];
	wchar_t rollbackFailure[96] = {0};

	if (feature >= TRAINER_FEATURE_COUNT) {
		report(TRAINER_ERROR, L"Invalid trainer feature.");
		return false;
	}
	if (!s_profileReady || s_memoryUncertain || !(s_snapshot.available & (1u << feature))) {
		report(TRAINER_ERROR, L"This action is unavailable: the game profile or memory state is not verified.");
		return false;
	}
	def = &s_features[feature];
	if (!read_feature(feature, before)) {
		clear_values();
		return false;
	}
	state = decode_feature(feature, before);
	if (state == TRAINER_UNKNOWN) {
		s_snapshot.states[feature] = TRAINER_UNKNOWN;
		s_snapshot.available = 0;
		report(TRAINER_UNSUPPORTED, L"Patch bytes changed externally. No writes were made.");
		return false;
	}
	target = trainer_next_state(feature, state, input);
	if (target == TRAINER_UNKNOWN) {
		report(TRAINER_ERROR, L"Invalid input for this trainer feature.");
		return false;
	}
	for (i = 0; i < def->count; ++i) {
		const PatchSite *site = &s_sites[def->first + i];
		site_bytes(site, target, after[i]);
		++attempted;
		if (!write_memory(site->rva, after[i], site->size)) {
			break;
		}
	}
	if (i == def->count) {
		uint8_t actual[2][MAX_PATCH_BYTES] = {{0}};
		if (read_feature(feature, actual)) {
			if (decode_feature(feature, actual) == target) {
				s_snapshot.states[feature] = target;
				report(TRAINER_CONNECTED, L"Connected to FFX.exe.");
				return true;
			}
			report(TRAINER_ERROR, L"Patch readback did not match the requested state.");
		}
	}
	memcpy(failure, s_snapshot.detail, sizeof failure);
	while (attempted) {
		const PatchSite *site;
		--attempted;
		site = &s_sites[def->first + attempted];
		if (!write_memory(site->rva, before[attempted], site->size)) {
			restored = false;
			if (!rollbackFailure[0]) {
				swprintf(rollbackFailure, sizeof rollbackFailure / sizeof *rollbackFailure,
					L"%.95ls", s_snapshot.detail);
			}
		}
	}
	{
		uint8_t actual[2][MAX_PATCH_BYTES] = {{0}};
		if (!read_feature(feature, actual)) {
			restored = false;
			if (!rollbackFailure[0]) {
				swprintf(rollbackFailure, sizeof rollbackFailure / sizeof *rollbackFailure,
					L"%.95ls", s_snapshot.detail);
			}
		} else if (decode_feature(feature, actual) != state) {
			restored = false;
			if (!rollbackFailure[0]) {
				swprintf(rollbackFailure, sizeof rollbackFailure / sizeof *rollbackFailure,
					L"Rollback readback mismatch.");
			}
		}
	}
	if (s_memoryUncertain && !rollbackFailure[0]) {
		swprintf(rollbackFailure, sizeof rollbackFailure / sizeof *rollbackFailure,
			L"Code protection restoration is uncertain.");
	}
	restored = restored && !s_memoryUncertain;
	s_snapshot.states[feature] = restored ? state : TRAINER_UNKNOWN;
	s_snapshot.available = 0;
	s_snapshot.status = TRAINER_ERROR;
	if (restored) {
		swprintf(s_snapshot.detail, sizeof s_snapshot.detail / sizeof *s_snapshot.detail,
			L"%.170ls Previous bytes restored; writes disabled until refresh.", failure);
	} else {
		swprintf(s_snapshot.detail, sizeof s_snapshot.detail / sizeof *s_snapshot.detail,
			L"%.85ls ROLLBACK FAILED: %.95ls Restart the game; memory may be partially patched.",
			failure, rollbackFailure);
	}
	return false;
}

#ifndef TRAINER_TEST
static HANDLE s_process;
static uintptr_t s_base;
static size_t s_imageSize;
static IMAGE_NT_HEADERS32 s_nt;
static IMAGE_SECTION_HEADER s_sections[96];

static void system_error(const wchar_t *operation, uint32_t rva, DWORD error) {
	wchar_t explanation[128] = {0};
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, 0, explanation, (DWORD)(sizeof explanation / sizeof *explanation), NULL);
	s_snapshot.status = TRAINER_ERROR;
	swprintf(s_snapshot.detail, sizeof s_snapshot.detail / sizeof *s_snapshot.detail,
		L"%ls at RVA 0x%lX (Windows %lu). %.127ls",
		operation, (unsigned long)rva, (unsigned long)error, explanation);
}

static void detach(void) {
	if (s_process) {
		CloseHandle(s_process);
	}
	s_process = NULL;
	s_base = 0;
	s_imageSize = 0;
	s_relocation = 0;
	s_profileReady = false;
	clear_values();
}

static bool read_memory(uint32_t rva, void *buffer, size_t size) {
	SIZE_T transferred = 0;
	size_t checked = 0;
	BOOL ok;
	if (!s_process || rva > s_imageSize || size > s_imageSize - rva) {
		system_error(L"Read outside game image", rva, ERROR_INVALID_ADDRESS);
		return false;
	}
	while (checked < size) {
		MEMORY_BASIC_INFORMATION info;
		uintptr_t address = s_base + rva + checked;
		size_t span;
		if (!VirtualQueryEx(s_process, (LPCVOID)address, &info, sizeof info)) {
			system_error(L"VirtualQueryEx before read", rva, GetLastError());
			return false;
		}
		if (info.State != MEM_COMMIT || (info.Protect & PAGE_GUARD) ||
			!(info.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
				PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
			system_error(L"Unreadable game page", rva, ERROR_NOACCESS);
			return false;
		}
		span = (uintptr_t)info.BaseAddress + info.RegionSize - address;
		if (!span) {
			system_error(L"Empty game memory region", rva, ERROR_INVALID_ADDRESS);
			return false;
		}
		checked += span < size - checked ? span : size - checked;
	}
	ok = ReadProcessMemory(s_process, (LPCVOID)(s_base + rva), buffer, size, &transferred);
	if (!ok || transferred != size) {
		system_error(L"ReadProcessMemory", rva, ok ? ERROR_PARTIAL_COPY : GetLastError());
		return false;
	}
	return true;
}

static bool write_memory(uint32_t rva, const void *buffer, size_t size) {
	struct {
		LPVOID address;
		SIZE_T size;
		DWORD protection;
	} regions[MAX_PATCH_BYTES];
	uintptr_t address = s_base + rva;
	size_t remaining = size;
	unsigned count = 0;
	SIZE_T transferred = 0;
	bool ok = true;
	bool attempted = false;
	wchar_t failure[256] = {0};
	wchar_t protectionFailure[128] = {0};
	uint8_t actual[MAX_PATCH_BYTES];

	if (!s_process || !size || size > MAX_PATCH_BYTES ||
		rva > s_imageSize || size > s_imageSize - rva) {
		system_error(L"Write outside game image", rva, ERROR_INVALID_ADDRESS);
		return false;
	}
	while (remaining) {
		MEMORY_BASIC_INFORMATION info;
		SIZE_T span;
		DWORD protection;
		if (!VirtualQueryEx(s_process, (LPCVOID)address, &info, sizeof info)) {
			system_error(L"VirtualQueryEx", rva, GetLastError());
			ok = false;
			break;
		}
		if (info.State != MEM_COMMIT || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
			system_error(L"Unusable code page", rva, ERROR_NOACCESS);
			ok = false;
			break;
		}
		span = (uintptr_t)info.BaseAddress + info.RegionSize - address;
		if (span > remaining) {
			span = remaining;
		}
		if (!span) {
			system_error(L"Empty code region", rva, ERROR_INVALID_ADDRESS);
			ok = false;
			break;
		}
		if (info.Protect != PAGE_EXECUTE_READWRITE &&
			!VirtualProtectEx(s_process, (LPVOID)address, span, PAGE_EXECUTE_READWRITE, &protection)) {
			system_error(L"VirtualProtectEx", rva, GetLastError());
			ok = false;
			break;
		}
		if (info.Protect != PAGE_EXECUTE_READWRITE) {
			regions[count].address = (LPVOID)address;
			regions[count].size = span;
			regions[count].protection = protection;
			++count;
		}
		address += span;
		remaining -= span;
	}
	if (ok) {
		BOOL written;
		attempted = true;
		written = WriteProcessMemory(s_process, (LPVOID)(s_base + rva), buffer, size, &transferred);
		if (!written || transferred != size) {
			system_error(L"WriteProcessMemory", rva, written ? ERROR_PARTIAL_COPY : GetLastError());
			ok = false;
		}
	}
	if (!ok) {
		memcpy(failure, s_snapshot.detail, sizeof failure);
	}
	while (count) {
		DWORD ignored;
		--count;
		if (!VirtualProtectEx(s_process, regions[count].address, regions[count].size,
			regions[count].protection, &ignored)) {
			DWORD error = GetLastError();
			s_memoryUncertain = true;
			system_error(L"Restore code protection", rva, error);
			swprintf(protectionFailure, sizeof protectionFailure / sizeof *protectionFailure,
				L"Code protection restoration failed (Windows %lu). Restart the game.", (unsigned long)error);
			if (ok) {
				memcpy(failure, s_snapshot.detail, sizeof failure);
			}
			ok = false;
		}
	}
	if (attempted && !FlushInstructionCache(s_process, (LPCVOID)(s_base + rva), size)) {
		system_error(L"FlushInstructionCache", rva, GetLastError());
		if (ok) {
			memcpy(failure, s_snapshot.detail, sizeof failure);
		}
		ok = false;
	}
	if (!ok) {
		if (protectionFailure[0]) {
			swprintf(s_snapshot.detail, sizeof s_snapshot.detail / sizeof *s_snapshot.detail,
				L"%.120ls %.127ls", failure, protectionFailure);
		} else {
			memcpy(s_snapshot.detail, failure, sizeof failure);
		}
		return false;
	}
	if (!read_memory(rva, actual, size)) {
		return false;
	}
	if (memcmp(actual, buffer, size) != 0) {
		report(TRAINER_ERROR, L"Written code did not match on readback.");
		return false;
	}
	return true;
}

static const IMAGE_SECTION_HEADER *image_section(uint32_t rva, size_t size, DWORD flags) {
	unsigned i;
	for (i = 0; i < s_nt.FileHeader.NumberOfSections; ++i) {
		const IMAGE_SECTION_HEADER *section = &s_sections[i];
		if (rva >= section->VirtualAddress && size <= section->Misc.VirtualSize &&
			rva - section->VirtualAddress <= section->Misc.VirtualSize - size &&
			(section->Characteristics & flags) == flags) {
			return section;
		}
	}
	return NULL;
}

static bool validate_image(void) {
	IMAGE_DOS_HEADER dos;
	IMAGE_NT_HEADERS32 nt;
	uint32_t sectionRva;
	unsigned i;
	s_profileReady = false;
	if (!read_memory(0, &dos, sizeof dos)) {
		return false;
	}
	if (dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < 0 ||
		(uint32_t)dos.e_lfanew > s_imageSize ||
		sizeof nt > s_imageSize - (uint32_t)dos.e_lfanew) {
		report(TRAINER_UNSUPPORTED, L"FFX.exe has an unsupported DOS/PE header.");
		return false;
	}
	if (!read_memory((uint32_t)dos.e_lfanew, &nt, sizeof nt)) {
		return false;
	}
	if (nt.Signature != IMAGE_NT_SIGNATURE || nt.FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
		nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
		nt.OptionalHeader.SizeOfImage != s_imageSize || s_base > UINT32_MAX ||
		s_imageSize > UINT32_MAX - s_base ||
		s_imageSize < CHARACTER_BLOCK + CHARACTER_BYTES ||
		nt.OptionalHeader.ImageBase != 0x400000 ||
		nt.FileHeader.SizeOfOptionalHeader != sizeof nt.OptionalHeader ||
		nt.FileHeader.NumberOfSections == 0 || nt.FileHeader.NumberOfSections > 96 ||
		nt.OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_BASERELOC) {
		report(TRAINER_UNSUPPORTED, L"Unsupported FFX.exe image/architecture. Expected the historical 32-bit Steam image.");
		return false;
	}
	s_nt = nt;
	sectionRva = (uint32_t)dos.e_lfanew + (uint32_t)sizeof nt;
	if (!read_memory(sectionRva, s_sections,
		(size_t)nt.FileHeader.NumberOfSections * sizeof *s_sections)) {
		return false;
	}
	for (i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
		if (s_sections[i].VirtualAddress > s_imageSize ||
			s_sections[i].Misc.VirtualSize > s_imageSize - s_sections[i].VirtualAddress) {
			report(TRAINER_UNSUPPORTED, L"Invalid game image sections.");
			return false;
		}
	}
	for (i = 0; i < SITE_COUNT; ++i) {
		if (!image_section(s_sites[i].rva - CONTEXT_MARGIN,
			(size_t)s_sites[i].size + CONTEXT_MARGIN * 2, IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE)) {
			report(TRAINER_UNSUPPORTED, L"A patch address is outside the game's executable sections.");
			return false;
		}
	}
	if (!image_section(BATTLES_RVA, 4, IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE) ||
		!image_section(CHARACTER_BLOCK, CHARACTER_BYTES, IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE)) {
		report(TRAINER_UNSUPPORTED, L"Statistics are outside the game's data sections.");
		return false;
	}
	s_relocation = (uint32_t)s_base - nt.OptionalHeader.ImageBase;
	return true;
}

static const uint8_t *file_rva(const uint8_t *file, size_t length, uint32_t rva, size_t size) {
	const IMAGE_SECTION_HEADER *section = image_section(rva, size, 0);
	size_t offset, raw;
	if (!section) {
		return NULL;
	}
	offset = rva - section->VirtualAddress;
	raw = section->PointerToRawData;
	if (offset > section->SizeOfRawData || size > section->SizeOfRawData - offset ||
		raw > length || offset > length - raw || size > length - raw - offset) {
		return NULL;
	}
	return file + raw + offset;
}

static bool relocate_context(const uint8_t *file, size_t length) {
	IMAGE_DATA_DIRECTORY directory = s_nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	const uint8_t *blocks;
	size_t offset = 0;
	if (!directory.Size) {
		return true;
	}
	blocks = file_rva(file, length, directory.VirtualAddress, directory.Size);
	if (!blocks) {
		return false;
	}
	while (offset < directory.Size) {
		IMAGE_BASE_RELOCATION block;
		size_t entry;
		if (sizeof block > directory.Size - offset) {
			return false;
		}
		memcpy(&block, blocks + offset, sizeof block);
		if (block.SizeOfBlock < sizeof block || block.SizeOfBlock > directory.Size - offset ||
			block.SizeOfBlock % sizeof(WORD)) {
			return false;
		}
		for (entry = sizeof block; entry < block.SizeOfBlock; entry += sizeof(WORD)) {
			WORD fixup;
			uint32_t target;
			unsigned i;
			memcpy(&fixup, blocks + offset + entry, sizeof fixup);
			if ((fixup >> 12) == IMAGE_REL_BASED_ABSOLUTE) {
				continue;
			}
			if (block.VirtualAddress > UINT32_MAX - (fixup & 0xFFFu)) {
				return false;
			}
			target = block.VirtualAddress + (fixup & 0xFFFu);
			for (i = 0; i < SITE_COUNT; ++i) {
				uint32_t start = s_sites[i].rva - CONTEXT_MARGIN;
				size_t size = (size_t)s_sites[i].size + CONTEXT_MARGIN * 2;
				if ((uint64_t)target + 4 > start && target < start + size) {
					const uint8_t *source;
					uint32_t value;
					unsigned byte;
					if ((fixup >> 12) != IMAGE_REL_BASED_HIGHLOW) {
						return false;
					}
					source = file_rva(file, length, target, sizeof value);
					if (!source) {
						return false;
					}
					memcpy(&value, source, sizeof value);
					value += s_relocation;
					for (byte = 0; byte < sizeof value; ++byte) {
						if ((uint64_t)target + byte >= start && (uint64_t)target + byte < start + size) {
							s_context[i][target + byte - start] = (uint8_t)(value >> (byte * 8));
						}
					}
				}
			}
		}
		offset += block.SizeOfBlock;
	}
	return true;
}

static bool validate_file(const wchar_t *path) {
	HANDLE handle, mapping = NULL;
	const uint8_t *file = NULL;
	LARGE_INTEGER size;
	IMAGE_DOS_HEADER dos;
	IMAGE_NT_HEADERS32 nt;
	size_t length, sectionOffset;
	unsigned i;
	bool valid = false;
	s_profileReady = false;
	handle = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		system_error(L"Open game executable", 0, GetLastError());
		return false;
	}
	if (!GetFileSizeEx(handle, &size)) {
		system_error(L"Get game executable size", 0, GetLastError());
		goto cleanup;
	}
	if (size.QuadPart <= 0 || (uint64_t)size.QuadPart > SIZE_MAX) {
		goto unsupported;
	}
	length = (size_t)size.QuadPart;
	mapping = CreateFileMappingW(handle, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!mapping) {
		system_error(L"Map game executable", 0, GetLastError());
		goto cleanup;
	}
	file = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
	if (!file) {
		system_error(L"Read game executable", 0, GetLastError());
		goto cleanup;
	}
	if (length < sizeof dos) {
		goto unsupported;
	}
	memcpy(&dos, file, sizeof dos);
	if (dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < 0 ||
		(size_t)dos.e_lfanew > length || sizeof nt > length - (size_t)dos.e_lfanew) {
		goto unsupported;
	}
	memcpy(&nt, file + dos.e_lfanew, sizeof nt);
	if (nt.Signature != IMAGE_NT_SIGNATURE ||
		memcmp(&nt.FileHeader, &s_nt.FileHeader, sizeof nt.FileHeader) != 0 ||
		memcmp(&nt.OptionalHeader, &s_nt.OptionalHeader, sizeof nt.OptionalHeader) != 0) {
		goto unsupported;
	}
	sectionOffset = (size_t)dos.e_lfanew + sizeof nt;
	if ((size_t)nt.FileHeader.NumberOfSections * sizeof *s_sections > length - sectionOffset ||
		memcmp(file + sectionOffset, s_sections,
			(size_t)nt.FileHeader.NumberOfSections * sizeof *s_sections) != 0) {
		goto unsupported;
	}
	for (i = 0; i < SITE_COUNT; ++i) {
		size_t span = (size_t)s_sites[i].size + CONTEXT_MARGIN * 2;
		const uint8_t *context = file_rva(file, length, s_sites[i].rva - CONTEXT_MARGIN, span);
		if (!context || memcmp(context + CONTEXT_MARGIN, s_sites[i].bytes[0], s_sites[i].size) != 0) {
			goto unsupported;
		}
		memcpy(s_context[i], context, span);
	}
	if (!relocate_context(file, length)) {
		goto unsupported;
	}
	valid = true;
	s_profileReady = true;
	goto cleanup;

unsupported:
	report(TRAINER_UNSUPPORTED, L"Unsupported or modified game executable. Expected the matching, unmodified Windows Steam FFX.exe.");
cleanup:
	if (file) {
		UnmapViewOfFile(file);
	}
	if (mapping) {
		CloseHandle(mapping);
	}
	CloseHandle(handle);
	return valid;
}

static bool attach(void) {
	HANDLE snapshot;
	PROCESSENTRY32W entry;
	MODULEENTRY32W module;
	DWORD pid = 0;
	BOOL more;

	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		system_error(L"Enumerate processes", 0, GetLastError());
		return false;
	}
	memset(&entry, 0, sizeof entry);
	entry.dwSize = sizeof entry;
	more = Process32FirstW(snapshot, &entry);
	while (more) {
		if (lstrcmpiW(entry.szExeFile, TRAINER_PROCESS_NAME) == 0) {
			if (pid) {
				CloseHandle(snapshot);
				report(TRAINER_ERROR, L"Multiple FFX.exe processes found. Close the extra game instance.");
				return false;
			}
			pid = entry.th32ProcessID;
		}
		more = Process32NextW(snapshot, &entry);
	}
	{
		DWORD error = GetLastError();
		CloseHandle(snapshot);
		if (error != ERROR_NO_MORE_FILES) {
			system_error(L"Enumerate processes", 0, error);
			return false;
		}
	}
	if (!pid) {
		report(TRAINER_WAITING, L"Waiting for FFX.exe. Start the game to connect automatically.");
		return false;
	}
	s_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ |
		PROCESS_VM_WRITE | PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, pid);
	if (!s_process) {
		system_error(L"Open FFX.exe (check process permissions)", 0, GetLastError());
		return false;
	}
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snapshot == INVALID_HANDLE_VALUE) {
		system_error(L"Enumerate FFX.exe modules", 0, GetLastError());
		detach();
		return false;
	}
	memset(&module, 0, sizeof module);
	module.dwSize = sizeof module;
	more = Module32FirstW(snapshot, &module);
	while (more) {
		if (lstrcmpiW(module.szModule, TRAINER_PROCESS_NAME) == 0) {
			s_base = (uintptr_t)module.modBaseAddr;
			s_imageSize = module.modBaseSize;
			break;
		}
		more = Module32NextW(snapshot, &module);
	}
	{
		DWORD error = more ? ERROR_SUCCESS : GetLastError();
		CloseHandle(snapshot);
		if (!s_base) {
			system_error(L"Find FFX.exe module", 0, error);
			detach();
			return false;
		}
	}
	if (!validate_image() || !validate_file(module.szExePath)) {
		detach();
		return false;
	}
	return true;
}

void trainer_refresh(TrainerSnapshot *snapshot) {
	if (s_process) {
		DWORD result = WaitForSingleObject(s_process, 0);
		if (result == WAIT_FAILED) {
			system_error(L"Check game process", 0, GetLastError());
			detach();
			*snapshot = s_snapshot;
			return;
		}
		if (result != WAIT_TIMEOUT) {
			detach();
			s_memoryUncertain = false;
		}
	}
	if (!s_process) {
		clear_values();
		if (!attach()) {
			*snapshot = s_snapshot;
			return;
		}
	}
	if (!read_snapshot() && !s_memoryUncertain) {
		detach();
	}
	*snapshot = s_snapshot;
}

bool trainer_action(unsigned feature, TrainerInput input, TrainerSnapshot *snapshot) {
	bool ok = apply_action(feature, input);
	*snapshot = s_snapshot;
	return ok;
}

void trainer_close(void) {
	detach();
}
#endif
