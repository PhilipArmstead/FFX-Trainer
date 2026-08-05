// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memory.h"

#ifdef ARCH_WIN
#include <memoryapi.h>
#include <minwindef.h>
#include <tlhelp32.h>


DECLSPEC_IMPORT WINBOOL WINAPI CloseHandle(HANDLE hObject);

uintptr_t getModuleBaseAddress(DWORD procId, const char *modName) {
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != ((HANDLE)(LONG_PTR) - 1)) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry)) {
			do {
				if (!strncasecmp(modEntry.szModule, modName, 255)) {
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

BOOL readFromMemory(HANDLE fd, DWORD base, DWORD address, SIZE_T size, BYTE *buffer) {
	SIZE_T *bytesRead = {0};
	return ReadProcessMemory(fd, (LPVOID)((uintptr_t)base + (uintptr_t)address), buffer, size, bytesRead);
}

WINBOOL writeToMemory(HANDLE fd, DWORD base, DWORD address, SIZE_T size, BYTE *buffer) {
	SIZE_T *bytesRead = {0};
	return WriteProcessMemory(fd, (LPVOID)((uintptr_t)base + (uintptr_t)address), buffer, size, bytesRead);
}

uint8_t readByte(HANDLE fd, DWORD base, DWORD address) {
	SIZE_T *bytesRead = {0};
	uint8_t byte;
	ReadProcessMemory(fd, (LPVOID)((uintptr_t)base + (uintptr_t)address), &byte, 1, bytesRead);

	return byte;
}

#else
#include <stdint.h>
#include <unistd.h>


void readFromMemory(const uint64_t fd, const uint64_t base, const uint64_t address, const int length, uint8_t *bytes) {
	lseek(fd, base + address, SEEK_SET);
	read(fd, bytes, length);
}

uint8_t readByte(const long fd, const uint64_t base, const uint64_t address) {
	uint8_t byte;
	lseek(fd, base + address, SEEK_SET);
	read(fd, &byte, 1);

	return byte;
}

void writeToMemory(
	const uint64_t fd,
	const uint64_t base,
	const uint32_t address,
	const int length,
	const uint8_t *bytes
) {
	lseek(fd, base + address, SEEK_SET);
	write(fd, bytes, length);
}
#endif
