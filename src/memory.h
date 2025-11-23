// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef _WIN32
#include <memoryapi.h>
#include <minwindef.h>
#include <tlhelp32.h>


uintptr_t getModuleBaseAddress(DWORD procId, const char *modName);
BOOL readFromMemory(HANDLE fd, DWORD base, DWORD address, SIZE_T size, BYTE *buffer);
WINBOOL writeToMemory(HANDLE fd, DWORD base, DWORD address, SIZE_T size, BYTE *buffer);
uint8_t readByte(HANDLE fd, DWORD base, DWORD address);
#else
#include "types.h"


void readFromMemory(uint64_t fd, uint64_t base, uint64_t address, int length, uint8_t *bytes);
uint8_t readByte(long fd, uint64_t base, uint64_t address);
void writeToMemory(uint64_t fd, uint64_t base, uint32_t address, int length, const uint8_t *bytes);
#endif
