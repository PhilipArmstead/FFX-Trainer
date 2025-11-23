// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <raylib.h>

#include "types.h"


void window_create(uint32_t width, uint32_t height, uint16_t fps, const char *title, Image appIcon);
void window_destroy(void);
void window_beforeDraw(void);
void window_afterDraw(void);
