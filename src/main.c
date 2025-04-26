// SPDX-FileCopyrightText: © 2025 Phil Armstead <philarmstead@mailbox.org>
// SPDX-License-Identifier: GPL-3.0-or-later


#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "maths.h"
#include "memory-constants.h"
#include "memory.h"
#include "process.h"
#include "types.h"
#include "window.h"
#include "assets/fonts/FreeSans.ttf.h"


typedef struct {
	char tidus[8];
	char yuna[8];
	char auron[8];
	char kimahri[8];
	char wakka[8];
	char lulu[8];
	char rikku[8];
} CharacterString;

int main() {
#ifdef IS_RELEASE
	SetTraceLogLevel(LOG_NONE);
#endif

#ifdef _WIN32
	int pid = 0;
	HANDLE fd = getProcessFileDescriptor(&pid);
#else
	int64_t fd = getProcessFileDescriptor();
#endif

	const uint16_t SCREEN_WIDTH = 640;
	const uint16_t SCREEN_HEIGHT = 420;
	const uint8_t FPS = 30;
	window_create(SCREEN_WIDTH, SCREEN_HEIGHT, FPS, "FFX Trainer");
	const Image icon = LoadImage("assets/icon.png");
	SetWindowIcon(icon);

	uint16_t framesSinceDataUpdate = 300;
	char battleCountString[8] = {0};
	CharacterString kills = {0};
	CharacterString victories = {0};

	/**
	 * Bit 1 = is perfect Tidus OD toggled?
	 * Bit 2 = is perfect Lulu OD toggled?
	 * Bit 3 = is perfect Auron OD toggled?
	 * Bit 4 = is steal success rate toggled?
	 * Bit 5 = is added steal toggled?
	 * Bit 6 = is guaranteed equipment drops toggled?
	 */
	uint8_t mask = 0;
	uint8_t rareStealSuccessValue = RARE_STEAL_CHANCE_ORIGINAL_2;
	uint8_t moreRareDropsValue = MORE_RARE_DROPS_ORIGINAL;
	Color stealSuccessRateColour = BLACK;
	Color guaranteedEquipmentColour = BLACK;
	Color rareStealSuccessRateColour = BLACK;
	Color addedStealColour = BLACK;
	Color moreRareDropsColour = BLACK;
	Color perfectFuryColour = BLACK;
	Color perfectBushidoColour = BLACK;
	Color perfectSwordplayColour = BLACK;

#ifdef _WIN32
	bool isGameRunning = fd != NULL;
#else
	bool isGameRunning = fd != -1;
#endif

#ifdef _WIN32
	if (isGameRunning) {
		base = getModuleBaseAddress(pid, "FFX.exe");
	}
#endif

	const uint16_t loadButtonWidth = 175;
	const Rectangle16 loadButtonRectangle = {
		.height = 36,
		.width = loadButtonWidth,
		.y = 16,
		.x = SCREEN_WIDTH - loadButtonWidth - 16
	};


	Font font = LoadFontFromMemory(".ttf", FreeSans_ttf, 0, 18, 0, 0);
	const float fontSize = (float)font.baseSize;
	const float rareStealTextWidth = MeasureTextEx(font, rareStealString, fontSize, 0).x;
	const float rareDropTextWidth = MeasureTextEx(font, rareDropString, fontSize, 0).x;
	const float fiftyPercentWidth = MeasureTextEx(font, "50%", fontSize, 0).x;
	const float hundredPercentWidth = MeasureTextEx(font, "100%", fontSize, 0).x;
	const float zeroPercentWidth = MeasureTextEx(font, "0%", fontSize, 0).x;
	const float battlesWidth = MeasureTextEx(font, battlesString, fontSize, 0).x;
	const float tidusKillsWidth = MeasureTextEx(font, tidusKillsString, fontSize, 0).x;
	const float tidusVictoriesWidth = MeasureTextEx(font, tidusVictoriesString, fontSize, 0).x;
	const float yunaKillsWidth = MeasureTextEx(font, yunaKillsString, fontSize, 0).x;
	const float yunaVictoriesWidth = MeasureTextEx(font, yunaVictoriesString, fontSize, 0).x;
	const float auronKillsWidth = MeasureTextEx(font, auronKillsString, fontSize, 0).x;
	const float auronVictoriesWidth = MeasureTextEx(font, auronVictoriesString, fontSize, 0).x;
	const float wakkaKillsWidth = MeasureTextEx(font, wakkaKillsString, fontSize, 0).x;
	const float wakkaVictoriesWidth = MeasureTextEx(font, wakkaVictoriesString, fontSize, 0).x;
	const float luluKillsWidth = MeasureTextEx(font, luluKillsString, fontSize, 0).x;
	const float luluVictoriesWidth = MeasureTextEx(font, luluVictoriesString, fontSize, 0).x;
	const float kimahriKillsWidth = MeasureTextEx(font, kimahriKillsString, fontSize, 0).x;
	const float kimahriVictoriesWidth = MeasureTextEx(font, kimahriVictoriesString, fontSize, 0).x;
	const float rikkuKillsWidth = MeasureTextEx(font, rikkuKillsString, fontSize, 0).x;
	const float rikkuVictoriesWidth = MeasureTextEx(font, rikkuVictoriesString, fontSize, 0).x;

	while (true) {
		uintptr_t base = 0;
		if (WindowShouldClose()) {
			break;
		}

		int keyPressed;
		do {
			keyPressed = GetKeyPressed();

			switch (keyPressed) {
				case '1': {
					uint8_t *bytes = mask & GUARANTEED_STEAL_TOGGLED
						? (uint8_t[4]){
							STEAL_CHANCE_ORIGINAL_0,
							STEAL_CHANCE_ORIGINAL_1,
							STEAL_CHANCE_ORIGINAL_2,
							STEAL_CHANCE_ORIGINAL_3
						}
						: (uint8_t[4]){
							STEAL_CHANCE_NEW_0,
							STEAL_CHANCE_NEW_1,
							STEAL_CHANCE_NEW_2,
							STEAL_CHANCE_NEW_3
						};
					writeToMemory(fd, base, STEAL_CHANCE_LOCATION, 4, bytes);;
					break;
				}
				case '2': {
					uint8_t bytes[3] = {RARE_STEAL_CHANCE_NEW_0, RARE_STEAL_CHANCE_NEW_1};
					if (rareStealSuccessValue == RARE_STEAL_CHANCE_ORIGINAL_2) {
						bytes[2] = RARE_STEAL_CHANCE_NEW_2_50_50;
					} else if (rareStealSuccessValue == RARE_STEAL_CHANCE_NEW_2_50_50) {
						bytes[0] = 0x39;
						bytes[2] = 0xc8;
						bytes[2] = NO_OP;
					} else if (rareStealSuccessValue == NO_OP) {
						bytes[2] = RARE_STEAL_CHANCE_NEW_2_NEVER;
					} else {
						bytes[0] = RARE_STEAL_CHANCE_ORIGINAL_0;
						bytes[1] = RARE_STEAL_CHANCE_ORIGINAL_1;
						bytes[2] = RARE_STEAL_CHANCE_ORIGINAL_2;
					}
					writeToMemory(fd, base, RARE_STEAL_CHANCE_LOCATION, 3, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '3': {
					uint8_t *bytes = mask & ADDED_STEAL_TOGGLED
						? (uint8_t[2]){
							ADDED_STEAL_ORIGINAL_0,
							ADDED_STEAL_ORIGINAL_1,
						}
						: (uint8_t[2]){NO_OP, NO_OP};
					writeToMemory(fd, base, ADDED_STEAL_LOCATION, 2, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '4': {
					uint8_t bytes[1] = {0};
					if (moreRareDropsValue == MORE_RARE_DROPS_ORIGINAL) {
						bytes[0] = MORE_RARE_DROPS_NEW_50_50;
					} else if (moreRareDropsValue == MORE_RARE_DROPS_NEW_50_50) {
						bytes[0] = MORE_RARE_DROPS_NEW_ALWAYS;
					} else if (moreRareDropsValue == MORE_RARE_DROPS_NEW_ALWAYS) {
						bytes[0] = MORE_RARE_DROPS_NEW_NEVER;
					} else {
						bytes[0] = MORE_RARE_DROPS_ORIGINAL;
					}
					writeToMemory(fd, base, MORE_RARE_DROPS_LOCATION, 1, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '5': {
					uint8_t *bytes = mask & GUARANTEED_EQUIPMENT_DROP_TOGGLED
						? (uint8_t[1]){ALWAYS_DROP_EQUIPMENT_ORIGINAL}
						: (uint8_t[1]){ALWAYS_DROP_EQUIPMENT_NEW};
					writeToMemory(fd, base, ALWAYS_DROP_EQUIPMENT_LOCATION, 1, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '6': {
					uint8_t *bytes = mask & PERFECT_TIDUS_OD_TOGGLED
						? (uint8_t[6]){TIDUS_PERFECT_LIMIT_ORIGINAL}
						: (uint8_t[6]){TIDUS_PERFECT_LIMIT_NEW};
					writeToMemory(fd, base, TIDUS_PERFECT_LIMIT_LOCATION, 6, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '7': {
					uint8_t *bytes = mask & PERFECT_AURON_OD_TOGGLED
						? (uint8_t[7]){AURON_PERFECT_LIMIT_ORIGINAL}
						: (uint8_t[7]){AURON_PERFECT_LIMIT_NEW};
					writeToMemory(fd, base, AURON_PERFECT_LIMIT_LOCATION, 7, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '8': {
					uint8_t *bytes = mask & PERFECT_LULU_OD_TOGGLED
						? (uint8_t[13]){LULU_PERFECT_LIMIT_ORIGINAL}
						: (uint8_t[13]){LULU_PERFECT_LIMIT_NEW};
					writeToMemory(fd, base, LULU_PERFECT_LIMIT_LOCATION, 13, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				default:
					break;
			}
		} while (keyPressed != 0);

		// Handle "Load game" button click
		if (!isGameRunning && IsMouseButtonPressed(0)) {
			const uint16_t mouseX = GetMouseX();
			const uint16_t mouseY = GetMouseY();

			if (mouseX > loadButtonRectangle.x &&
				mouseX < loadButtonRectangle.x + loadButtonRectangle.width &&
				mouseY > loadButtonRectangle.y &&
				mouseY < loadButtonRectangle.y + loadButtonRectangle.height) {
#ifdef _WIN32
				fd = getProcessFileDescriptor(&pid);
				isGameRunning = fd != NULL;
				if (isGameRunning) {
					base = getModuleBaseAddress(pid, "FFX.exe");
				}
#else
				fd = getProcessFileDescriptor();
				isGameRunning = fd != -1;
#endif
			}
		}

		window_beforeDraw();

		if (!isGameRunning) {
			DrawRectangle(
				loadButtonRectangle.x,
				loadButtonRectangle.y,
				loadButtonRectangle.width,
				loadButtonRectangle.height,
				LIGHTGRAY
			);
			DrawText("Load game", loadButtonRectangle.x + 8, loadButtonRectangle.y + 4, 30, BLACK);

			DrawText(
				"Could not open game",
				loadButtonRectangle.x + 8 + 20,
				loadButtonRectangle.y + loadButtonRectangle.height + 4,
				12,
				RED
			);
		} else {
			if (framesSinceDataUpdate > FPS * 5) {
				// These numbers can't both be > 0 and equal.
				// If they are, it means we've failed to read memory correctly
				uint8_t buffer[4];
				readFromMemory(fd, base, TOTAL_BATTLES_LOCATION, 4, buffer);
				const uint64_t battleCount = hexBytesToInt(buffer, 4);
				readFromMemory(fd, base, YUNA_VICTORIES_LOCATION, 4, buffer);
				const uint64_t yunaVictories = hexBytesToInt(buffer, 4);

				if (battleCount > 0 && battleCount == yunaVictories) {
					isGameRunning = false;
					continue;
				}

				snprintf(battleCountString, 8, "%lu", battleCount);
				readFromMemory(fd, base, TIDUS_KILLS_LOCATION, 4, buffer);
				snprintf(kills.tidus, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, TIDUS_VICTORIES_LOCATION, 4, buffer);
				snprintf(victories.tidus, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, YUNA_KILLS_LOCATION, 4, buffer);
				snprintf(kills.yuna, 8, "%lu", hexBytesToInt(buffer, 4));
				snprintf(victories.yuna, 8, "%lu", yunaVictories);
				readFromMemory(fd, base, AURON_KILLS_LOCATION, 4, buffer);
				snprintf(kills.auron, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, AURON_VICTORIES_LOCATION, 4, buffer);
				snprintf(victories.auron, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, KIMAHRI_KILLS_LOCATION, 4, buffer);
				snprintf(kills.kimahri, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, KIMAHRI_VICTORIES_LOCATION, 4, buffer);
				snprintf(victories.kimahri, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, RIKKU_KILLS_LOCATION, 4, buffer);
				readFromMemory(fd, base, WAKKA_KILLS_LOCATION, 4, buffer);
				snprintf(kills.wakka, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, WAKKA_VICTORIES_LOCATION, 4, buffer);
				snprintf(victories.wakka, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, LULU_KILLS_LOCATION, 4, buffer);
				snprintf(kills.lulu, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, LULU_VICTORIES_LOCATION, 4, buffer);
				snprintf(victories.lulu, 8, "%lu", hexBytesToInt(buffer, 4));
				snprintf(kills.rikku, 8, "%lu", hexBytesToInt(buffer, 4));
				readFromMemory(fd, base, RIKKU_VICTORIES_LOCATION, 4, buffer);
				snprintf(victories.rikku, 8, "%lu", hexBytesToInt(buffer, 4));

				readFromMemory(fd, base, STEAL_CHANCE_LOCATION, 4, buffer);
				if (
					buffer[0] != STEAL_CHANCE_ORIGINAL_0 ||
					buffer[1] != STEAL_CHANCE_ORIGINAL_1 ||
					buffer[2] != STEAL_CHANCE_ORIGINAL_2 ||
					buffer[3] != STEAL_CHANCE_ORIGINAL_3
				) {
					mask |= GUARANTEED_STEAL_TOGGLED;
				} else {
					mask &= ~GUARANTEED_STEAL_TOGGLED;
				}
				readFromMemory(fd, base, RARE_STEAL_CHANCE_LOCATION + 2, 1, buffer);
				rareStealSuccessValue = buffer[0];
				readFromMemory(fd, base, ADDED_STEAL_LOCATION, 3, buffer);
				if (buffer[0] != ADDED_STEAL_ORIGINAL_0 || buffer[1] != ADDED_STEAL_ORIGINAL_1) {
					mask |= ADDED_STEAL_TOGGLED;
				} else {
					mask &= ~ADDED_STEAL_TOGGLED;
				}
				readFromMemory(fd, base, MORE_RARE_DROPS_LOCATION, 1, buffer);
				moreRareDropsValue = buffer[0];
				readFromMemory(fd, base, TIDUS_PERFECT_LIMIT_LOCATION + 5, 1, buffer);
				if (buffer[0] == NO_OP) {
					mask |= PERFECT_TIDUS_OD_TOGGLED;
				} else {
					mask &= ~PERFECT_TIDUS_OD_TOGGLED;
				}
				readFromMemory(fd, base, LULU_PERFECT_LIMIT_LOCATION + 7, 1, buffer);
				if (buffer[0] == NO_OP) {
					mask |= PERFECT_LULU_OD_TOGGLED;
				} else {
					mask &= ~PERFECT_LULU_OD_TOGGLED;
				}
				readFromMemory(fd, base, AURON_PERFECT_LIMIT_LOCATION + 5, 1, buffer);
				if (buffer[0] == NO_OP) {
					mask |= PERFECT_AURON_OD_TOGGLED;
				} else {
					mask &= ~PERFECT_AURON_OD_TOGGLED;
				}
				readFromMemory(fd, base, ALWAYS_DROP_EQUIPMENT_LOCATION, 1, buffer);
				if (buffer[0] != ALWAYS_DROP_EQUIPMENT_ORIGINAL) {
					mask |= GUARANTEED_EQUIPMENT_DROP_TOGGLED;
				} else {
					mask &= ~GUARANTEED_EQUIPMENT_DROP_TOGGLED;
				}

				stealSuccessRateColour = mask & GUARANTEED_STEAL_TOGGLED ? GREEN : BLACK;
				addedStealColour = mask & ADDED_STEAL_TOGGLED ? GREEN : BLACK;
				rareStealSuccessRateColour = rareStealSuccessValue != RARE_STEAL_CHANCE_ORIGINAL_2 ? GREEN : BLACK;
				guaranteedEquipmentColour = mask & GUARANTEED_EQUIPMENT_DROP_TOGGLED ? GREEN : BLACK;
				moreRareDropsColour = moreRareDropsValue != MORE_RARE_DROPS_ORIGINAL ? GREEN : BLACK;
				perfectFuryColour = mask & PERFECT_LULU_OD_TOGGLED ? GREEN : BLACK;
				perfectBushidoColour = mask & PERFECT_AURON_OD_TOGGLED ? GREEN : BLACK;
				perfectSwordplayColour = mask & PERFECT_TIDUS_OD_TOGGLED ? GREEN : BLACK;

				framesSinceDataUpdate = 0;
			}

			// Print game stats
			const Rectangle16 dataRectangle = {
				.height = 18,
				.y = loadButtonRectangle.y,
				.x = SCREEN_WIDTH - 130
			};
			const float statsRight = (float)SCREEN_WIDTH - STATS_RIGHT;
			const float valueX = statsRight + 8;
			DrawTextEx(font, battlesString, (Vector2){statsRight - battlesWidth, dataRectangle.y}, fontSize, 0, BLACK);
			DrawTextEx(font, battleCountString, (Vector2){valueX, dataRectangle.y}, fontSize, 0, BLACK);

			uint8_t i = 1;
			DrawTextEx(
				font,
				tidusKillsString,
				(Vector2){statsRight - tidusKillsWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kills.tidus,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				SKYBLUE
			);
			DrawTextEx(
				font,
				tidusVictoriesString,
				(Vector2){statsRight - tidusVictoriesWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				victories.tidus,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				SKYBLUE
			);
			DrawTextEx(
				font,
				yunaKillsString,
				(Vector2){statsRight - yunaKillsWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kills.yuna,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				GRAY
			);
			DrawTextEx(
				font,
				yunaVictoriesString,
				(Vector2){statsRight - yunaVictoriesWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				victories.yuna,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				GRAY
			);
			DrawTextEx(
				font,
				auronKillsString,
				(Vector2){statsRight - auronKillsWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kills.auron,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				RED
			);
			DrawTextEx(
				font,
				auronVictoriesString,
				(Vector2){statsRight - auronVictoriesWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				victories.auron,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				RED
			);
			DrawTextEx(
				font,
				wakkaKillsString,
				(Vector2){statsRight - wakkaKillsWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kills.wakka,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				ORANGE
			);
			DrawTextEx(
				font,
				wakkaVictoriesString,
				(Vector2){statsRight - wakkaVictoriesWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				victories.wakka,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				ORANGE
			);
			DrawTextEx(
				font,
				luluKillsString,
				(Vector2){statsRight - luluKillsWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kills.lulu,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				luluVictoriesString,
				(Vector2){statsRight - luluVictoriesWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				victories.lulu,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kimahriKillsString,
				(Vector2){statsRight - kimahriKillsWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kills.kimahri,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				BLUE
			);
			DrawTextEx(
				font,
				kimahriVictoriesString,
				(Vector2){statsRight - kimahriVictoriesWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				victories.kimahri,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				BLUE
			);
			DrawTextEx(
				font,
				rikkuKillsString,
				(Vector2){statsRight - rikkuKillsWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				kills.rikku,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				GREEN
			);
			DrawTextEx(
				font,
				rikkuVictoriesString,
				(Vector2){statsRight - rikkuVictoriesWidth, (float)(dataRectangle.y + (dataRectangle.height + 8) * i)},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				victories.rikku,
				(Vector2){valueX, (float)(dataRectangle.y + (dataRectangle.height + 8) * i++)},
				fontSize,
				0,
				GREEN
			);
		}

		// Draw hack texts
		uint8_t y = 1;
		DrawTextEx(
			font,
			perfectStealString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			stealSuccessRateColour
		);
		DrawTextEx(
			font,
			rareStealString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			rareStealSuccessRateColour
		);
		DrawTextEx(
			font,
			addedStealString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			addedStealColour
		);
		DrawTextEx(
			font,
			rareDropString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			moreRareDropsColour
		);
		DrawTextEx(
			font,
			alwaysDropEquipmentString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			guaranteedEquipmentColour
		);
		DrawTextEx(
			font,
			perfectSwordplayString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			perfectSwordplayColour
		);
		DrawTextEx(
			font,
			perfectBushidoString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			perfectBushidoColour
		);
		DrawTextEx(
			font,
			perfectFuryString,
			(Vector2){PADDING_LEFT, (float)y++ * LINE_HEIGHT},
			fontSize,
			0,
			perfectFuryColour
		);

		if (rareStealSuccessValue != RARE_STEAL_CHANCE_ORIGINAL_2) {
			DrawTextEx(font, "(", (Vector2){PADDING_LEFT + rareStealTextWidth + 6, 50}, fontSize, 0, BLACK);
			DrawTextEx(
				font,
				")",
				(Vector2){
					PADDING_LEFT + rareStealTextWidth + fiftyPercentWidth + hundredPercentWidth + zeroPercentWidth + 22,
					50
				},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				"50%",
				(Vector2){PADDING_LEFT + rareStealTextWidth + 12, 50},
				fontSize,
				0,
				rareStealSuccessValue == RARE_STEAL_CHANCE_NEW_2_50_50 ? GREEN : BLACK
			);
			DrawTextEx(
				font,
				"100%",
				(Vector2){PADDING_LEFT + rareStealTextWidth + fiftyPercentWidth + 16, 50},
				fontSize,
				0,
				rareStealSuccessValue == NO_OP ? GREEN : BLACK
			);
			DrawTextEx(
				font,
				"0%",
				(Vector2){PADDING_LEFT + rareStealTextWidth + fiftyPercentWidth + hundredPercentWidth + 22, 50},
				fontSize,
				0,
				rareStealSuccessValue == RARE_STEAL_CHANCE_NEW_2_NEVER ? GREEN : BLACK
			);
		}

		if (moreRareDropsValue != MORE_RARE_DROPS_ORIGINAL) {
			DrawTextEx(font, "(", (Vector2){PADDING_LEFT + rareDropTextWidth + 6, 102}, fontSize, 0, BLACK);
			DrawTextEx(
				font,
				")",
				(Vector2){
					PADDING_LEFT + rareDropTextWidth + fiftyPercentWidth + hundredPercentWidth + zeroPercentWidth + 22,
					102 // TODO: derive this the same way we get the position of the cheat label
				},
				fontSize,
				0,
				BLACK
			);
			DrawTextEx(
				font,
				"50%",
				(Vector2){PADDING_LEFT + rareDropTextWidth + 12, 102},
				fontSize,
				0,
				moreRareDropsValue == MORE_RARE_DROPS_NEW_50_50 ? GREEN : BLACK
			);
			DrawTextEx(
				font,
				"100%",
				(Vector2){PADDING_LEFT + rareDropTextWidth + fiftyPercentWidth + 16, 102},
				fontSize,
				0,
				moreRareDropsValue == MORE_RARE_DROPS_NEW_ALWAYS ? GREEN : BLACK
			);
			DrawTextEx(
				font,
				"0%",
				(Vector2){PADDING_LEFT + rareDropTextWidth + fiftyPercentWidth + hundredPercentWidth + 22, 102},
				fontSize,
				0,
				moreRareDropsValue == MORE_RARE_DROPS_NEW_NEVER ? GREEN : BLACK
			);
		}

		window_afterDraw();

		++framesSinceDataUpdate;
	}

	UnloadFont(font);
	UnloadImage(icon);

	window_destroy();

	return 0;
}
