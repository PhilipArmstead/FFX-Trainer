// SPDX-License-Identifier: GPL-3.0-or-later

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include "trainer_fixture.h"
#include "../src/main.c"

static bool s_attached = true;

void trainer_refresh(TrainerSnapshot *snapshot) {
	if (s_attached) {
		assert(read_snapshot());
	} else {
		clear_values();
		report(TRAINER_WAITING, L"Waiting for the fixture.");
	}
	*snapshot = s_snapshot;
}

bool trainer_action(unsigned feature, TrainerInput input, TrainerSnapshot *snapshot) {
	bool ok = apply_action(feature, input);
	*snapshot = s_snapshot;
	return ok;
}

void trainer_close(void) {
	clear_values();
	s_profileReady = false;
}

static void click(HWND hwnd, int row, unsigned char kind) {
	int run;
	for (run = 0; run < g_rowLayout[row].nRuns; ++run) {
		const Run *layout = &g_rowLayout[row].runs[run];
		if (layout->kind == kind) {
			int x = layout->x + layout->w / 2, y = g_rowLayout[row].y + 1;
			assert(HitTest(x, y, NULL) == row);
			SendMessageW(hwnd, WM_LBUTTONDOWN, 0, MAKELPARAM(x, y));
			return;
		}
	}
	assert(false);
}

static void check_alignment(HWND hwnd) {
	int i;
	HDC dc = GetDC(hwnd);
	HFONT previous = (HFONT)SelectObject(dc, g_font);
	for (i = 0; i < STAT_COUNT; ++i) {
		SIZE extent;
		assert(GetTextExtentPoint32W(dc, g_statValues[i], lstrlenW(g_statValues[i]), &extent));
		assert(g_statLayout[i].xValue + extent.cx == g_cx - SCALE(PAD));
	}
	SelectObject(dc, previous);
	ReleaseDC(hwnd, dc);
}

int main(void) {
	WNDCLASSW cls;
	HINSTANCE instance = GetModuleHandleW(NULL);
	HWND hwnd;
	RECT bounds;
	unsigned row, state, writes;
	uint32_t value = UINT32_MAX;
	reset();
	InitDpi();
	CreateFonts();
	g_bgBrush = CreateSolidBrush(COL_BG);
	g_arrow = LoadCursor(NULL, IDC_ARROW);
	g_hand = LoadCursor(NULL, IDC_HAND);
	memset(&cls, 0, sizeof cls);
	cls.lpfnWndProc = WndProc;
	cls.hInstance = instance;
	cls.lpszClassName = APP_CLASS;
	assert(RegisterClassW(&cls));
	SetRect(&bounds, 0, 0, SCALE(DESIGN_W), SCALE(DESIGN_H));
	AdjustForDpi(&bounds, WINDOW_STYLE);
	hwnd = CreateWindowExW(0, APP_CLASS, APP_TITLE, WINDOW_STYLE, 0, 0,
		bounds.right - bounds.left, bounds.bottom - bounds.top, NULL, NULL, instance, NULL);
	assert(hwnd && g_snapshot.available == UINT8_MAX);
	for (row = 0; row < ROW_COUNT; ++row) {
		for (state = 1; state <= s_features[row].states; ++state) {
			SendMessageW(hwnd, WM_KEYDOWN, '1' + row, 0);
			assert(g_snapshot.states[row] == state % s_features[row].states);
		}
		SendMessageW(hwnd, WM_KEYDOWN, VK_NUMPAD1 + row, 0);
		assert(g_snapshot.states[row] == 1);
		click(hwnd, (int)row, KIND_LABEL);
		assert(g_snapshot.states[row] == 0);
		click(hwnd, (int)row, KIND_LABEL);
		assert(g_snapshot.states[row] == 1);
		click(hwnd, (int)row, KIND_LABEL);
		assert(g_snapshot.states[row] == 0);
	}
	for (row = 1; row <= 3; row += 2) {
		const Run *hidden = &g_rowLayout[row].runs[4];
		assert(HitTest(hidden->x + hidden->w / 2, g_rowLayout[row].y + 1, NULL) == -1);
		click(hwnd, (int)row, KIND_LABEL);
		click(hwnd, (int)row, KIND_NUM1);
		assert(g_snapshot.states[row] == 2);
		click(hwnd, (int)row, KIND_NUM2);
		assert(g_snapshot.states[row] == 3);
		click(hwnd, (int)row, KIND_NUM0);
		assert(g_snapshot.states[row] == 1);
		click(hwnd, (int)row, KIND_NUM0);
		assert(g_snapshot.states[row] == 0);
	}
	memcpy(s_memory + BATTLES_RVA, &value, sizeof value);
	SendMessageW(hwnd, WM_TIMER, 1, 0);
	assert(wcscmp(g_statValues[0], L"4294967295") == 0);
	check_alignment(hwnd);
	value = 0;
	memcpy(s_memory + BATTLES_RVA, &value, sizeof value);
	SendMessageW(hwnd, WM_TIMER, 1, 0);
	assert(wcscmp(g_statValues[0], L"0") == 0);
	check_alignment(hwnd);
	GetWindowRect(hwnd, &bounds);
	SendMessageW(hwnd, WM_DPICHANGED, MAKELONG(144, 144), (LPARAM)&bounds);
	assert(g_dpi == 144);
	check_alignment(hwnd);
	{
		HDC screen = GetDC(hwnd), dc = CreateCompatibleDC(screen);
		HBITMAP bitmap = CreateCompatibleBitmap(screen, g_cx, g_cy);
		HBITMAP previous = (HBITMAP)SelectObject(dc, bitmap);
		assert(bitmap);
		DrawContent(dc);
		SelectObject(dc, previous);
		DeleteObject(bitmap);
		DeleteDC(dc);
		ReleaseDC(hwnd, screen);
	}
	s_attached = false;
	SendMessageW(hwnd, WM_TIMER, 1, 0);
	assert(g_snapshot.available == 0 && wcscmp(g_statValues[0], L"--") == 0);
	assert(HitTest(g_rowLayout[0].runs[0].x + 1, g_rowLayout[0].y + 1, NULL) == -1);
	writes = s_writes;
	SendMessageW(hwnd, WM_KEYDOWN, '1', 0);
	assert(s_writes == writes);
	s_attached = true;
	reset();
	SendMessageW(hwnd, WM_TIMER, 1, 0);
	assert(g_snapshot.states[0] == 0 && s_writes == 0);
	SendMessageW(hwnd, WM_KEYDOWN, '1', 0);
	SendMessageW(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
	assert(!IsWindow(hwnd));
	assert(memcmp(s_memory + s_sites[0].rva, s_sites[0].bytes[1], s_sites[0].size) == 0);
	DeleteObject(g_font);
	DeleteObject(g_fontBold);
	DeleteObject(g_bgBrush);
	assert(UnregisterClassW(APP_CLASS, instance));
	puts("Native Win32 input, live-stat layout, DPI and lifecycle checks passed.");
	return 0;
}
