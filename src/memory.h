// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later


#pragma once

#ifdef _WIN32

#include <memoryapi.h>
#include <minwindef.h>
#include <tlhelp32.h>


#define INVALID_HANDLE_VALUE ((HANDLE) (LONG_PTR)-1)
#define WINBASEAPI DECLSPEC_IMPORT

WINBASEAPI WINBOOL WINAPI CloseHandle(HANDLE hObject);


uintptr_t getModuleBaseAddress(DWORD procId, const char *modName) {
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry)) {
			do {
				if (!strncasecmp(modEntry.szModule, modName, 255)) {
					modBaseAddr = (uintptr_t) modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

static inline BOOL readFromMemory(HANDLE fd, DWORD base, DWORD address, SIZE_T size, BYTE *buffer) {
	SIZE_T *bytesRead = {0};
	return ReadProcessMemory(fd, (LPVOID) (base + address), buffer, size, bytesRead);
}

static inline WINBOOL writeToMemory(HANDLE fd, DWORD base, DWORD address, SIZE_T size, BYTE *buffer) {
	SIZE_T *bytesRead = {0};
	return WriteProcessMemory(fd, (LPVOID) (base + address), buffer, size, bytesRead);
}

static inline uint8_t readByte(HANDLE fd, DWORD base, DWORD address) {
	SIZE_T *bytesRead = {0};
	uint8_t byte;
	ReadProcessMemory(fd, (LPVOID) (base + address), &byte, 1, bytesRead);

	return byte;
}

#else
#include <stdint.h>
#include <unistd.h>


static inline void readFromMemory(const long fd, const uint64_t base, const uint64_t address, const int length, uint8_t *bytes) {
	lseek(fd, base + address, SEEK_SET);
	read(fd, bytes, length);
}

static inline uint8_t readByte(const long fd, const uint64_t base, uint64_t address) {
	uint8_t byte;
	lseek(fd, base + address, SEEK_SET);
	read(fd, &byte, 1);

	return byte;
}

static inline void writeToMemory(
	const long fd,
	const uint64_t base,
	const uint32_t address,
	const int length,
	const uint8_t *bytes
) {
	lseek(fd, base + address, SEEK_SET);
	write(fd, bytes, length);
}
#endif
