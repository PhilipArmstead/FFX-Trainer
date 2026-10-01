// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#define WIN32_LEAN_AND_MEAN
#define WINVER       0x0601
#define _WIN32_WINNT 0x0601

#include <stdint.h>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#define APP_CLASS    L"FfxTrainerWindow"
#define APP_TITLE    L"FFX Trainer"
#define VERSION_TEXT L"v0.13.0"
#define IDI_APP_ICON 1

#define WINDOW_STYLE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

/* Colours sampled from the reference screenshot. */
#define COL_BG     RGB(246, 245, 244)
#define COL_OFF    RGB( 16,  16,  16)
#define COL_ON     RGB(  0, 200,  59)
#define COL_LABEL  RGB( 45,  51,  53)
#define COL_BLUE   RGB( 74, 163, 255)
#define COL_GREY   RGB(135, 136, 135)
#define COL_RED    RGB(255,  47,  47)
#define COL_ORANGE RGB(245, 168,  40)
#define COL_GREEN  RGB( 60, 212,  86)

/* Design metrics in 96-dpi pixels; scaled at runtime. */
enum {
	DESIGN_W   = 580,
	DESIGN_H   = 449,
	PAD        = 24,
	TOP        = 30,
	LINE_LEFT  = 25,
	LINE_RIGHT = 27,
	FONT_PX    = 15,
	ROW_COUNT  = 8,
	STAT_COUNT = 15,
	MAX_RUNS   = 8,
	LINE_CHARS = 64
};

/* Run kinds. KIND_NUM0..2 map onto row states 1..3. */
enum { KIND_PUNCT = 0, KIND_LABEL = 1, KIND_NUM0 = 2, KIND_NUM1 = 3, KIND_NUM2 = 4 };

typedef struct {
	const wchar_t *text;
	BOOL cyclic;
} RowDef;

typedef struct {
	const wchar_t *label;
	const wchar_t *value;
	COLORREF color;
} StatDef;

typedef struct {
	int len;
	unsigned char kind;
} SegDef;

typedef struct {
	const wchar_t *text;
	int len;
	int x;
	int w;
	unsigned char kind;
} Run;

typedef struct {
	wchar_t line[LINE_CHARS];
	Run runs[MAX_RUNS];
	int nRuns;
	int y;
	RECT bounds;
} RowLayout;

typedef struct {
	int y;
	int xLabel;
	int xValue;
	int lenLabel;
	int lenValue;
} StatLayout;

static const RowDef g_rowDefs[ROW_COUNT] = {
	{L"1) Toggle 100% steal chance", FALSE},
	{L"2) Toggle rare steal chance", TRUE},
	{L"3) Toggle added steal", FALSE},
	{L"4) Toggle rare drop chance", TRUE},
	{L"5) Toggle always drop equipment", FALSE},
	{L"6) Toggle perfect Swordplay", FALSE},
	{L"7) Toggle perfect Bushido", FALSE},
	{L"8) Toggle perfect Fury", FALSE}
};

static const wchar_t g_extra[] = L" (50% 100% 0%)";

static const SegDef g_extraSegs[] = {
	{2, KIND_PUNCT},
	/* " ("  */
	{3, KIND_NUM0},
	/* "50%" */
	{1, KIND_PUNCT},
	/* " "   */
	{4, KIND_NUM1},
	/* "100%"*/
	{1, KIND_PUNCT},
	/* " "   */
	{2, KIND_NUM2},
	/* "0%"  */
	{1, KIND_PUNCT} /* ")"   */
};

#define EXTRA_SEG_COUNT ((int)(sizeof g_extraSegs / sizeof g_extraSegs[0]))

static const StatDef g_statDefs[STAT_COUNT] = {
	{L"Battles:", L"65", COL_LABEL},
	{L"Tidus kills:", L"81", COL_BLUE},
	{L"Tidus victories:", L"63", COL_BLUE},
	{L"Yuna kills:", L"4", COL_GREY},
	{L"Yuna victories:", L"31", COL_GREY},
	{L"Auron kills:", L"7", COL_RED},
	{L"Auron victories:", L"5", COL_RED},
	{L"Wakka kills:", L"24", COL_ORANGE},
	{L"Wakka victories:", L"37", COL_ORANGE},
	{L"Lulu kills:", L"18", COL_GREY},
	{L"Lulu victories:", L"18", COL_GREY},
	{L"Kimahri kills:", L"3", COL_BLUE},
	{L"Kimahri victories:", L"7", COL_BLUE},
	{L"Rikku kills:", L"1", COL_GREEN},
	{L"Rikku victories:", L"20", COL_GREEN}
};

static uint8_t g_state[ROW_COUNT];
static RowLayout g_rowLayout[ROW_COUNT];
static StatLayout g_statLayout[STAT_COUNT];
static int g_versionX, g_versionY;
static int g_textH = 0;
static int g_cx = 0, g_cy = 0;
static int g_dpi = 96;
static HFONT g_font = NULL, g_fontBold = NULL;
static HBRUSH g_bgBrush = NULL;
static HCURSOR g_hand = NULL, g_arrow = NULL;

#define SCALE(v) MulDiv((v), g_dpi, 96)

typedef BOOL (WINAPI *PFN_SetProcessDpiAwarenessContext)(HANDLE);
typedef UINT (WINAPI *PFN_GetDpiForSystem)(void);
typedef BOOL (WINAPI *PFN_AdjustWindowRectExForDpi)(LPRECT, DWORD, BOOL, DWORD, UINT);

static PFN_AdjustWindowRectExForDpi s_AdjustWindowRectExForDpi = NULL;

static void InitDpi(void) {
	HMODULE user32 = GetModuleHandleW(L"user32.dll");
	PFN_SetProcessDpiAwarenessContext setContext;
	PFN_GetDpiForSystem getSystemDpi;

	if (!user32) {
		return;
	}

	setContext = (PFN_SetProcessDpiAwarenessContext)(void (*)(void))
		GetProcAddress(user32, "SetProcessDpiAwarenessContext");
	getSystemDpi = (PFN_GetDpiForSystem)(void (*)(void))
		GetProcAddress(user32, "GetDpiForSystem");
	s_AdjustWindowRectExForDpi = (PFN_AdjustWindowRectExForDpi)(void (*)(void))
		GetProcAddress(user32, "AdjustWindowRectExForDpi");

	/* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 */
	if (!setContext || !setContext((HANDLE)(INT_PTR)-4)) {
		SetProcessDPIAware();
	}

	if (getSystemDpi) {
		g_dpi = (int)getSystemDpi();
	} else {
		HDC screen = GetDC(NULL);
		if (screen) {
			g_dpi = GetDeviceCaps(screen, LOGPIXELSX);
			ReleaseDC(NULL, screen);
		}
	}
	if (g_dpi <= 0) {
		g_dpi = 96;
	}
}

static void AdjustForDpi(RECT *r, DWORD style) {
	if (s_AdjustWindowRectExForDpi) {
		s_AdjustWindowRectExForDpi(r, style, FALSE, 0, (UINT)g_dpi);
	} else {
		AdjustWindowRectEx(r, style, FALSE, 0);
	}
}

static void CreateFonts(void) {
	LOGFONTW lf;

	if (g_font) {
		DeleteObject(g_font);
	}
	if (g_fontBold) {
		DeleteObject(g_fontBold);
	}

	ZeroMemory(&lf, sizeof lf);
	lf.lfHeight = -SCALE(FONT_PX);
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_TT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	lstrcpynW(lf.lfFaceName, L"Segoe UI", LF_FACESIZE);

	g_font = CreateFontIndirectW(&lf);
	lf.lfWeight = FW_BOLD;
	g_fontBold = CreateFontIndirectW(&lf);
}

static void BuildLayout(HWND hwnd) {
	HDC hdc = GetDC(hwnd);
	HFONT prevFont;
	TEXTMETRICW tm;
	SIZE sz, space;
	int pad, top, lineL, lineR, right, i, s;

	if (!hdc || g_cx <= 0 || g_cy <= 0) {
		if (hdc) {
			ReleaseDC(hwnd, hdc);
		}
		return;
	}

	prevFont = (HFONT)SelectObject(hdc, g_font);
	GetTextMetricsW(hdc, &tm);
	g_textH = tm.tmHeight;

	pad = SCALE(PAD);
	top = SCALE(TOP);
	lineL = SCALE(LINE_LEFT);
	lineR = SCALE(LINE_RIGHT);
	right = g_cx - pad;

	for (i = 0; i < ROW_COUNT; ++i) {
		RowLayout *rl = &g_rowLayout[i];
		const RowDef *rd = &g_rowDefs[i];
		int labelLen, offset, n;

		wsprintfW(rl->line, L"%s%s", rd->text, rd->cyclic ? g_extra : L"");
		labelLen = lstrlenW(rd->text);
		rl->y = top + i * lineL;

		GetTextExtentPoint32W(hdc, rl->line, labelLen, &sz);
		rl->runs[0].text = rl->line;
		rl->runs[0].len = labelLen;
		rl->runs[0].x = pad;
		rl->runs[0].w = sz.cx;
		rl->runs[0].kind = KIND_LABEL;
		n = 1;
		offset = labelLen;

		if (rd->cyclic) {
			for (s = 0; s < EXTRA_SEG_COUNT; ++s) {
				SIZE start;
				GetTextExtentPoint32W(hdc, rl->line, offset, &start);
				GetTextExtentPoint32W(hdc, rl->line, offset + g_extraSegs[s].len, &sz);
				rl->runs[n].text = rl->line + offset;
				rl->runs[n].len = g_extraSegs[s].len;
				rl->runs[n].x = pad + start.cx;
				rl->runs[n].w = sz.cx - start.cx;
				rl->runs[n].kind = g_extraSegs[s].kind;
				offset += g_extraSegs[s].len;
				++n;
			}
		}
		rl->nRuns = n;

		GetTextExtentPoint32W(hdc, rl->line, offset, &sz);
		SetRect(&rl->bounds, pad, rl->y, pad + sz.cx + 1, rl->y + tm.tmHeight);
	}

	GetTextExtentPoint32W(hdc, L" ", 1, &space);

	for (i = 0; i < STAT_COUNT; ++i) {
		StatLayout *sl = &g_statLayout[i];
		SIZE labelSz, valueSz;

		sl->lenLabel = lstrlenW(g_statDefs[i].label);
		sl->lenValue = lstrlenW(g_statDefs[i].value);
		GetTextExtentPoint32W(hdc, g_statDefs[i].label, sl->lenLabel, &labelSz);
		GetTextExtentPoint32W(hdc, g_statDefs[i].value, sl->lenValue, &valueSz);

		sl->y = top + i * lineR;
		sl->xValue = right - valueSz.cx;
		sl->xLabel = sl->xValue - space.cx - labelSz.cx;
	}

	g_versionX = pad;
	g_versionY = top + (STAT_COUNT - 1) * lineR;

	SelectObject(hdc, prevFont);
	ReleaseDC(hwnd, hdc);
}

static void DrawContent(HDC hdc) {
	HFONT prevFont;
	RECT rc;
	int i, r;

	SetRect(&rc, 0, 0, g_cx, g_cy);
	FillRect(hdc, &rc, g_bgBrush);

	SetBkMode(hdc, TRANSPARENT);
	prevFont = (HFONT)SelectObject(hdc, g_font);

	for (i = 0; i < ROW_COUNT; ++i) {
		const RowLayout *rl = &g_rowLayout[i];
		unsigned state = g_state[i];
		int count = state ? rl->nRuns : 1;

		for (r = 0; r < count; ++r) {
			const Run *run = &rl->runs[r];
			COLORREF color;

			switch (run->kind) {
				case KIND_LABEL:
					color = state ? COL_ON : COL_OFF;
					break;
				case KIND_PUNCT:
					color = COL_OFF;
					break;
				default:
					color = (state == (unsigned)(run->kind - 1)) ? COL_ON : COL_OFF;
					break;
			}
			SetTextColor(hdc, color);
			TextOutW(hdc, run->x, rl->y, run->text, run->len);
		}
	}

	for (i = 0; i < STAT_COUNT; ++i) {
		const StatLayout *sl = &g_statLayout[i];

		SetTextColor(hdc, COL_LABEL);
		TextOutW(hdc, sl->xLabel, sl->y, g_statDefs[i].label, sl->lenLabel);
		SetTextColor(hdc, g_statDefs[i].color);
		TextOutW(hdc, sl->xValue, sl->y, g_statDefs[i].value, sl->lenValue);
	}

	SelectObject(hdc, g_fontBold);
	SetTextColor(hdc, COL_LABEL);
	TextOutW(
		hdc,
		g_versionX,
		g_versionY,
		VERSION_TEXT,
		(int)(sizeof VERSION_TEXT / sizeof(wchar_t)) - 1
	);

	SelectObject(hdc, prevFont);
}

/* Returns the row index under (x, y) and writes the run index, or -1. */
static int HitTest(int x, int y, int *outRun) {
	int i, r;

	for (i = 0; i < ROW_COUNT; ++i) {
		const RowLayout *rl = &g_rowLayout[i];
		unsigned state = g_state[i];
		int count;

		if (y < rl->y || y >= rl->y + g_textH) {
			continue;
		}
		count = state ? rl->nRuns : 1;
		for (r = 0; r < count; ++r) {
			const Run *run = &rl->runs[r];
			if (run->kind == KIND_PUNCT) {
				continue;
			}
			if (x >= run->x && x < run->x + run->w) {
				if (outRun) {
					*outRun = r;
				}
				return i;
			}
		}
		return -1;
	}
	return -1;
}

static void ApplyRun(int row, unsigned char kind) {
	if (kind == KIND_LABEL) {
		g_state[row] = g_state[row] ? 0 : 1;
	} else {
		uint8_t n = (uint8_t)(kind - 1);
		g_state[row] = (g_state[row] == n) ? 0 : n;
	}
}

static void ApplyKey(int row) {
	if (g_rowDefs[row].cyclic) {
		g_state[row] = (uint8_t)((g_state[row] + 1u) & 3u);
	} else {
		g_state[row] ^= 1u;
	}
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CREATE: {
			RECT rc;
			GetClientRect(hwnd, &rc);
			g_cx = rc.right;
			g_cy = rc.bottom;
			BuildLayout(hwnd);
			return 0;
		}

		case WM_SIZE:
			g_cx = (int)(short)LOWORD(lParam);
			g_cy = (int)(short)HIWORD(lParam);
			BuildLayout(hwnd);
			InvalidateRect(hwnd, NULL, FALSE);
			return 0;

		case WM_ERASEBKGND:
			return 1;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			int w = ps.rcPaint.right - ps.rcPaint.left;
			int h = ps.rcPaint.bottom - ps.rcPaint.top;

			if (hdc && w > 0 && h > 0) {
				HDC mem = CreateCompatibleDC(hdc);
				HBITMAP bmp = mem ? CreateCompatibleBitmap(hdc, w, h) : NULL;

				if (bmp) {
					HBITMAP prevBmp = (HBITMAP)SelectObject(mem, bmp);
					SetViewportOrgEx(mem, -ps.rcPaint.left, -ps.rcPaint.top, NULL);
					DrawContent(mem);
					SetViewportOrgEx(mem, 0, 0, NULL);
					BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, w, h, mem, 0, 0, SRCCOPY);
					SelectObject(mem, prevBmp);
					DeleteObject(bmp);
				} else {
					DrawContent(hdc);
				}
				if (mem) {
					DeleteDC(mem);
				}
			}
			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_LBUTTONDOWN: {
			int run = 0;
			int row = HitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &run);

			SetFocus(hwnd);
			if (row >= 0) {
				ApplyRun(row, g_rowLayout[row].runs[run].kind);
				InvalidateRect(hwnd, &g_rowLayout[row].bounds, FALSE);
			}
			return 0;
		}

		case WM_SETCURSOR:
			if (LOWORD(lParam) == HTCLIENT) {
				POINT pt;
				if (GetCursorPos(&pt) && ScreenToClient(hwnd, &pt)) {
					SetCursor(HitTest(pt.x, pt.y, NULL) >= 0 ? g_hand : g_arrow);
					return TRUE;
				}
			}
			break;

		case WM_KEYDOWN: {
			int row = -1;

			if (wParam >= '1' && wParam <= '8') {
				row = (int)wParam - '1';
			} else if (wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8) {
				row = (int)wParam - VK_NUMPAD1;
			}
			if (row >= 0) {
				ApplyKey(row);
				InvalidateRect(hwnd, &g_rowLayout[row].bounds, FALSE);
				return 0;
			}
			if (wParam == VK_ESCAPE) {
				DestroyWindow(hwnd);
				return 0;
			}
			break;
		}

		case WM_DPICHANGED: {
			const RECT *suggested = (const RECT*)lParam;
			RECT rc;

			g_dpi = (int)HIWORD(wParam);
			CreateFonts();

			SetRect(&rc, 0, 0, SCALE(DESIGN_W), SCALE(DESIGN_H));
			AdjustForDpi(&rc, WINDOW_STYLE);
			SetWindowPos(
				hwnd,
				NULL,
				suggested->left,
				suggested->top,
				rc.right - rc.left,
				rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE
			);
			return 0;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmdLine, int showCmd) {
	WNDCLASSEXW wc;
	RECT rc;
	HWND hwnd;
	MSG msg;

	(void)hPrev;
	(void)cmdLine;

	InitDpi();
	CreateFonts();
	g_bgBrush = CreateSolidBrush(COL_BG);
	g_arrow = LoadCursor(NULL, IDC_ARROW);
	g_hand = LoadCursor(NULL, IDC_HAND);

	ZeroMemory(&wc, sizeof wc);
	wc.cbSize = sizeof wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = (HICON)LoadImageW(
		hInstance,
		MAKEINTRESOURCEW(IDI_APP_ICON),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON),
		LR_DEFAULTCOLOR
	);
	wc.hIconSm = (HICON)LoadImageW(
		hInstance,
		MAKEINTRESOURCEW(IDI_APP_ICON),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR
	);
	wc.hCursor = g_arrow;
	wc.hbrBackground = NULL;
	wc.lpszClassName = APP_CLASS;
	if (!RegisterClassExW(&wc)) {
		return 1;
	}

	SetRect(&rc, 0, 0, SCALE(DESIGN_W), SCALE(DESIGN_H));
	AdjustForDpi(&rc, WINDOW_STYLE);

	hwnd = CreateWindowExW(
		0,
		APP_CLASS,
		APP_TITLE,
		WINDOW_STYLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hwnd) {
		return 1;
	}

	ShowWindow(hwnd, showCmd);
	UpdateWindow(hwnd);

	ZeroMemory(&msg, sizeof msg);
	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	if (g_font) {
		DeleteObject(g_font);
	}
	if (g_fontBold) {
		DeleteObject(g_fontBold);
	}
	if (g_bgBrush) {
		DeleteObject(g_bgBrush);
	}
	return (int)msg.wParam;
}
