#pragma once

#include <raylib.h>


static inline void window_create(const uint32_t width, const uint32_t height, const uint16_t fps, const char *title) {
  InitWindow(width, height, title);
  SetTargetFPS(fps);
}

static inline void window_destroy() {
  CloseWindow();
}

static inline void window_beforeDraw() {
  BeginDrawing();
  ClearBackground(RAYWHITE);
}

static inline void window_afterDraw() {
  EndDrawing();
}
