// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"


void window_create(
  const uint32_t width,
  const uint32_t height,
  const uint16_t fps,
  const char *title,
  const Image appIcon
) {
  InitWindow(width, height, title);
  SetTargetFPS(fps);
  SetWindowIcon(appIcon);
}

void window_destroy(void) {
  CloseWindow();
}

void window_beforeDraw(void) {
  BeginDrawing();
  ClearBackground(RAYWHITE);
}

void window_afterDraw(void) {
  EndDrawing();
}
