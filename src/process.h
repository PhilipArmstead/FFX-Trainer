// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later


#pragma once

#ifdef _WIN32

#include <minwindef.h>
#include <stdio.h>
#include <tlhelp32.h>


#define INVALID_HANDLE_VALUE ((HANDLE) (LONG_PTR)-1)
#define WINBASEAPI DECLSPEC_IMPORT

WINBASEAPI WINBOOL WINAPI CloseHandle(HANDLE hObject);

WINBASEAPI HANDLE WINAPI OpenProcess(DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwProcessId);

static inline int findProcessByName(const char *processName) {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) { return 0; }

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	BOOL hResult = Process32First(hSnapshot, &pe);

	int pid = 0;
	while (hResult) {
		if (strcmp(processName, pe.szExeFile) == 0) {
			pid = pe.th32ProcessID;
			break;
		}
		hResult = Process32Next(hSnapshot, &pe);
	}

	CloseHandle(hSnapshot);
	return pid;
}

static inline HANDLE getProcessFileDescriptor(int *pid) {
	*pid = findProcessByName("FFX.exe");
	return OpenProcess(PROCESS_ALL_ACCESS, TRUE, *pid);
}

#else
#include <fcntl.h>
#include <stdlib.h>


static inline long openProcess() {
	FILE *fp = popen("pidof FFX.exe", "r");
	char path[64];
	fgets(path, sizeof(path), fp);
	const long pid = strtol(path, NULL, 10);
	fclose(fp);

	return pid;
}

static inline int64_t getProcessFileDescriptor() {
	const long pid = openProcess();
	if (pid) {
		char memoryPath[32];
		sprintf(memoryPath, "/proc/%ld/mem", pid);
		return open(memoryPath, O_RDWR);
	}

	return -1;
}
#endif
