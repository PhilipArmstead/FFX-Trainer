#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "maths.h"
#include "memory.h"
#include "process.h"
#include "types.h"
#include "window.h"


int main() {
	int fd = getProcessFileDescriptor();

	const uint16_t SCREEN_WIDTH = 640;
	const uint16_t SCREEN_HEIGHT = 480;
	const uint8_t FPS = 30;
	window_create(SCREEN_WIDTH, SCREEN_HEIGHT, FPS, "FFX Trainer");

	uint16_t framesSinceDataUpdate = 300;
	char battleCountString[8];
	char tidusKillCountString[8];
	char tidusVictoriesCountString[8];
	char rikkuKillCountString[8];
	char rikkuVictoriesCountString[8];
	bool isStealSuccessRateToggled = false;
	bool isRareStealSuccessRateToggled = false;
	bool isAddedStealToggled = false;
	bool isPerfectFuryToggled = false;
	Color stealSuccessRateColour = isStealSuccessRateToggled ? GREEN : BLACK;
	Color rareStealSuccessRateColour = isRareStealSuccessRateToggled ? GREEN : BLACK;
	Color addedStealColour = isAddedStealToggled ? GREEN : BLACK;
	Color perfectFuryColour = isPerfectFuryToggled ? GREEN : BLACK;

	const uint16_t loadButtonWidth = 175;
	const Rectangle16 loadButtonRectangle = {
		.height = 36,
		.width = loadButtonWidth,
		.y = 16,
		.x = SCREEN_WIDTH - loadButtonWidth - 16
	};

	while (true) {
		if (WindowShouldClose()) {
			break;
		}

		int keyPressed;
		do {
			keyPressed = GetKeyPressed();

			switch (keyPressed) {
				case '1': {
					const uint8_t *bytes = isStealSuccessRateToggled
						                       ? (const uint8_t[4]){
							                       STEAL_CHANCE_ORIGINAL_0,
							                       STEAL_CHANCE_ORIGINAL_1,
							                       STEAL_CHANCE_ORIGINAL_2,
							                       STEAL_CHANCE_ORIGINAL_3
						                       }
						                       : (const uint8_t[4]){
							                       STEAL_CHANCE_NEW_0,
							                       STEAL_CHANCE_NEW_1,
							                       STEAL_CHANCE_NEW_2,
							                       STEAL_CHANCE_NEW_3
						                       };
					writeToMemory(fd, STEAL_CHANCE_LOCATION, 4, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '2': {
					const uint8_t *bytes = isRareStealSuccessRateToggled
						                       ? (const uint8_t[3]){
							                       RARE_STEAL_CHANCE_ORIGINAL_0,
							                       RARE_STEAL_CHANCE_ORIGINAL_1,
							                       RARE_STEAL_CHANCE_ORIGINAL_2,
						                       }
						                       : (const uint8_t[3]){
							                       RARE_STEAL_CHANCE_NEW_0,
							                       RARE_STEAL_CHANCE_NEW_1,
							                       RARE_STEAL_CHANCE_NEW_2,
						                       };
					writeToMemory(fd, RARE_STEAL_CHANCE_LOCATION, 3, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '3': {
					const uint8_t *bytes = isAddedStealToggled
						                       ? (const uint8_t[2]){
							                       ADDED_STEAL_ORIGINAL_0,
							                       ADDED_STEAL_ORIGINAL_1,
						                       }
						                       : (const uint8_t[2]){
							                       ADDED_STEAL_NEW_0,
							                       ADDED_STEAL_NEW_1,
						                       };
					writeToMemory(fd, ADDED_STEAL_LOCATION, 2, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '4': {
					const uint8_t *bytes = isPerfectFuryToggled
						                       ? (const uint8_t[1]){LULU_STARTING_FURY_COUNT_ORIGINAL_0}
						                       : (const uint8_t[1]){LULU_STARTING_FURY_COUNT_NEW_0};
					writeToMemory(fd, LULU_STARTING_FURY_COUNT_LOCATION, 1, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				default:
					break;
			}
		} while (keyPressed != 0);

		// Handle "Load game" button click
		if (IsMouseButtonPressed(0)) {
			const uint16_t mouseX = GetMouseX();
			const uint16_t mouseY = GetMouseY();

			if (mouseX > loadButtonRectangle.x && mouseX < loadButtonRectangle.x + loadButtonRectangle.width && mouseY >
				loadButtonRectangle.y && mouseY < loadButtonRectangle.y + loadButtonRectangle.height) {
				fd = getProcessFileDescriptor();
			}
		}

		window_beforeDraw();

		// Draw "Load game" button
		DrawRectangle(
			loadButtonRectangle.x,
			loadButtonRectangle.y,
			loadButtonRectangle.width,
			loadButtonRectangle.height,
			LIGHTGRAY
		);
		DrawText("Load game", loadButtonRectangle.x + 8, loadButtonRectangle.y + 4, 30, BLACK);

		if (fd == -1) {
			DrawText(
				"Could not open game",
				loadButtonRectangle.x + 8 + 20,
				loadButtonRectangle.y + loadButtonRectangle.height + 4,
				12,
				RED
			);
		}

		// Update game data
		if (fd != -1 && framesSinceDataUpdate > FPS * 5) {
			framesSinceDataUpdate = 0;

			uint8_t buffer[4];
			readFromMemory(fd, TOTAL_BATTLES_LOCATION, 4, buffer);
			snprintf(battleCountString, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, TIDUS_KILLS_LOCATION, 4, buffer);
			snprintf(tidusKillCountString, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, TIDUS_VICTORIES_LOCATION, 4, buffer);
			snprintf(tidusVictoriesCountString, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, RIKKU_KILLS_LOCATION, 4, buffer);
			snprintf(rikkuKillCountString, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, RIKKU_VICTORIES_LOCATION, 4, buffer);
			snprintf(rikkuVictoriesCountString, 8, "%lu", hexBytesToInt(buffer, 4));

			readFromMemory(fd, STEAL_CHANCE_LOCATION, 4, buffer);
			isStealSuccessRateToggled =
				buffer[0] != STEAL_CHANCE_ORIGINAL_0 ||
				buffer[1] != STEAL_CHANCE_ORIGINAL_1 ||
				buffer[2] != STEAL_CHANCE_ORIGINAL_2 ||
				buffer[3] != STEAL_CHANCE_ORIGINAL_3;
			readFromMemory(fd, RARE_STEAL_CHANCE_LOCATION, 3, buffer);
			isRareStealSuccessRateToggled =
				buffer[0] != RARE_STEAL_CHANCE_ORIGINAL_0 ||
				buffer[1] != RARE_STEAL_CHANCE_ORIGINAL_1 ||
				buffer[2] != RARE_STEAL_CHANCE_ORIGINAL_2;
			readFromMemory(fd, ADDED_STEAL_LOCATION, 3, buffer);
			isAddedStealToggled =
				buffer[0] != ADDED_STEAL_ORIGINAL_0 ||
				buffer[1] != ADDED_STEAL_ORIGINAL_1;
			readFromMemory(fd, LULU_STARTING_FURY_COUNT_LOCATION, 1, buffer);
			isPerfectFuryToggled = buffer[0] != LULU_STARTING_FURY_COUNT_ORIGINAL_0;

			stealSuccessRateColour = isStealSuccessRateToggled ? GREEN : BLACK;
			rareStealSuccessRateColour = isRareStealSuccessRateToggled ? GREEN : BLACK;
			addedStealColour = isAddedStealToggled ? GREEN : BLACK;
			perfectFuryColour = isPerfectFuryToggled ? GREEN : BLACK;
		}
		// Print game data
		const Rectangle16 dataRectangle = {
			.height = 18,
			.width = 62,
			.y = loadButtonRectangle.y + loadButtonRectangle.height + 16,
			.x = SCREEN_WIDTH - 130
		};
		DrawText("Battles:", dataRectangle.x, dataRectangle.y, 16, BLACK);
		DrawText(battleCountString, dataRectangle.x + dataRectangle.width, dataRectangle.y, 16, BLACK);
		DrawText("Tidus kills:", dataRectangle.x - 20, dataRectangle.y + dataRectangle.height + 8, 16, BLACK);
		DrawText(
			tidusKillCountString,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + dataRectangle.height + 8,
			16,
			BLUE
		);
		DrawText("Tidus victories:", dataRectangle.x - 60, dataRectangle.y + (dataRectangle.height + 8) * 2, 16, BLACK);
		DrawText(
			tidusVictoriesCountString,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * 2,
			16,
			BLUE
		);
		DrawText("Rikku kills:", dataRectangle.x - 20, dataRectangle.y + (dataRectangle.height + 8) * 3, 16, BLACK);
		DrawText(
			rikkuKillCountString,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * 3,
			16,
			GREEN
		);
		DrawText("Rikku victories:", dataRectangle.x - 60, dataRectangle.y + (dataRectangle.height + 8) * 4, 16, BLACK);
		DrawText(
			rikkuVictoriesCountString,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * 4,
			16,
			GREEN
		);

		// Draw hack texts
		DrawText("1) Toggle 100% steal chance", 24, 24, 16, stealSuccessRateColour);
		DrawText("2) Toggle rare steal chance", 24, 48, 16, rareStealSuccessRateColour);
		DrawText("3) Toggle added steal", 24, 72, 16, addedStealColour);
		DrawText("4) Toggle perfect fury", 24, 96, 16, perfectFuryColour);

		window_afterDraw();

		++framesSinceDataUpdate;
	}

	window_destroy();

	return 0;
}
