// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define VERSION_MAJOR 0
#define VERSION_MINOR 12
#define VERSION_PATCH 0

#define PERFECT_TIDUS_OD_TOGGLED (1 << 0)
#define PERFECT_LULU_OD_TOGGLED (1 << 1)
#define PERFECT_AURON_OD_TOGGLED (1 << 2)
#define GUARANTEED_STEAL_TOGGLED (1 << 3)
#define ADDED_STEAL_TOGGLED (1 << 4)
#define GUARANTEED_EQUIPMENT_DROP_TOGGLED (1 << 5)

#ifdef ARCH_WIN
#define LONG_SPECIFIER "%llu"
#else
#define LONG_SPECIFIER "%lu"
#endif
