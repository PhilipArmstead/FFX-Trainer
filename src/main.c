#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "maths.h"
#include "memory.h"
#include "process.h"
#include "types.h"
#include "window.h"


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
	int fd = getProcessFileDescriptor();

	const uint16_t SCREEN_WIDTH = 640;
	const uint16_t SCREEN_HEIGHT = 480;
	const uint8_t FPS = 30;
	window_create(SCREEN_WIDTH, SCREEN_HEIGHT, FPS, "FFX Trainer");

	uint16_t framesSinceDataUpdate = 300;
	char battleCountString[8] = {0};
	CharacterString kills = {0};
	CharacterString victories = {0};
	bool isStealSuccessRateToggled = false;
	bool isRareStealSuccessRateToggled = false;
	bool isAddedStealToggled = false;
	bool areMoreRareDropsToggled = false;
	bool isPerfectFuryToggled = false;
	Color stealSuccessRateColour = isStealSuccessRateToggled ? GREEN : BLACK;
	Color rareStealSuccessRateColour = isRareStealSuccessRateToggled ? GREEN : BLACK;
	Color addedStealColour = isAddedStealToggled ? GREEN : BLACK;
	Color moreRareDropsColour = areMoreRareDropsToggled ? GREEN : BLACK;
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
					const uint8_t *bytes = areMoreRareDropsToggled
						                       ? (const uint8_t[1]){MORE_RARE_DROPS_ORIGINAL_0}
						                       : (const uint8_t[1]){MORE_RARE_DROPS_NEW_0};
					writeToMemory(fd, MORE_RARE_DROPS_LOCATION, 1, bytes);
					framesSinceDataUpdate = FPS * 5;
					break;
				}
				case '5': {
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
			snprintf(kills.tidus, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, TIDUS_VICTORIES_LOCATION, 4, buffer);
			snprintf(victories.tidus, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, YUNA_KILLS_LOCATION, 4, buffer);
			snprintf(kills.yuna, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, YUNA_VICTORIES_LOCATION, 4, buffer);
			snprintf(victories.yuna, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, AURON_KILLS_LOCATION, 4, buffer);
			snprintf(kills.auron, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, AURON_VICTORIES_LOCATION, 4, buffer);
			snprintf(victories.auron, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, KIMAHRI_KILLS_LOCATION, 4, buffer);
			snprintf(kills.kimahri, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, KIMAHRI_VICTORIES_LOCATION, 4, buffer);
			snprintf(victories.kimahri, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, RIKKU_KILLS_LOCATION, 4, buffer);
			readFromMemory(fd, WAKKA_KILLS_LOCATION, 4, buffer);
			snprintf(kills.wakka, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, WAKKA_VICTORIES_LOCATION, 4, buffer);
			snprintf(victories.wakka, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, LULU_KILLS_LOCATION, 4, buffer);
			snprintf(kills.lulu, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, LULU_VICTORIES_LOCATION, 4, buffer);
			snprintf(victories.lulu, 8, "%lu", hexBytesToInt(buffer, 4));
			snprintf(kills.rikku, 8, "%lu", hexBytesToInt(buffer, 4));
			readFromMemory(fd, RIKKU_VICTORIES_LOCATION, 4, buffer);
			snprintf(victories.rikku, 8, "%lu", hexBytesToInt(buffer, 4));

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
			readFromMemory(fd, MORE_RARE_DROPS_LOCATION, 1, buffer);
			areMoreRareDropsToggled = buffer[0] != MORE_RARE_DROPS_ORIGINAL_0;
			readFromMemory(fd, LULU_STARTING_FURY_COUNT_LOCATION, 1, buffer);
			isPerfectFuryToggled = buffer[0] != LULU_STARTING_FURY_COUNT_ORIGINAL_0;

			stealSuccessRateColour = isStealSuccessRateToggled ? GREEN : BLACK;
			rareStealSuccessRateColour = isRareStealSuccessRateToggled ? GREEN : BLACK;
			addedStealColour = isAddedStealToggled ? GREEN : BLACK;
			moreRareDropsColour = areMoreRareDropsToggled ? GREEN : BLACK;
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

		uint8_t i = 1;
		DrawText("Tidus kills:", dataRectangle.x - 20, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			kills.tidus,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			SKYBLUE
		);
		DrawText("Tidus victories:", dataRectangle.x - 60, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			victories.tidus,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			SKYBLUE
		);
		DrawText("Yuna kills:", dataRectangle.x - 20, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			kills.yuna,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			GRAY
		);
		DrawText("Yuna victories:", dataRectangle.x - 60, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			victories.yuna,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			GRAY
		);
		DrawText("Auron kills:", dataRectangle.x - 30, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			kills.auron,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			RED
		);
		DrawText("Auron victories:", dataRectangle.x - 70, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			victories.auron,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			RED
		);
		DrawText("Wakka kills:", dataRectangle.x - 30, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			kills.wakka,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			ORANGE
		);
		DrawText("Wakka victories:", dataRectangle.x - 68, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			victories.wakka,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			ORANGE
		);
		DrawText("Lulu kills:", dataRectangle.x - 15, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			kills.lulu,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			BLACK
		);
		DrawText("Lulu victories:", dataRectangle.x - 55, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			victories.lulu,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			BLACK
		);
		DrawText("Kimahri kills:", dataRectangle.x - 35, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			kills.kimahri,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			BLUE
		);
		DrawText("Kimahri victories:", dataRectangle.x - 75, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			victories.kimahri,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			BLUE
		);
		DrawText("Rikku kills:", dataRectangle.x - 20, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			kills.rikku,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			GREEN
		);
		DrawText("Rikku victories:", dataRectangle.x - 60, dataRectangle.y + (dataRectangle.height + 8) * i, 16, BLACK);
		DrawText(
			victories.rikku,
			dataRectangle.x + dataRectangle.width,
			dataRectangle.y + (dataRectangle.height + 8) * i++,
			16,
			GREEN
		);

		// Draw hack texts
		DrawText("1) Toggle 100% steal chance", 24, 24, 16, stealSuccessRateColour);
		DrawText("2) Toggle rare steal chance", 24, 48, 16, rareStealSuccessRateColour);
		DrawText("3) Toggle added steal", 24, 72, 16, addedStealColour);
		DrawText("4) Toggle more rare drops", 24, 96, 16, moreRareDropsColour);
		DrawText("5) Toggle perfect fury", 24, 120, 16, perfectFuryColour);

		window_afterDraw();

		++framesSinceDataUpdate;
	}

	window_destroy();

	return 0;
}
