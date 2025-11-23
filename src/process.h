// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef _WIN32
#include <minwindef.h>
#include <stdio.h>
#include <tlhelp32.h>


int findProcessByName(const char *processName);
HANDLE getProcessFileDescriptor(int *pid);

#else
#include "types.h"


long openProcess(void);
int64_t getProcessFileDescriptor(void);
#endif
