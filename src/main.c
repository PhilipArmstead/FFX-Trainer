#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "constants.h"
#include "memory.h"
#include "types.h"

int main() {
	FILE *fp = popen("pidof FFX.exe", "r");
	char path[64];
	fgets(path, sizeof(path), fp);
	const long pid = strtol(path, NULL, 10);
	fclose(fp);
	if (!pid) {
		printf("Couldn't find process; is it running?\n");
		exit(1);
	}

	char memoryPath[32];
	sprintf(memoryPath, "/proc/%ld/mem", pid);
	const int fd = open(memoryPath, O_RDWR);
	if (fd == -1) {
		printf("Couldn't open %s\n", memoryPath);
		exit(2);
	}

	while (1) {
		// Check if we have enabled scripts
		uint8_t buffer[4];
		readFromMemory(fd, STEAL_CHANCE_LOCATION, 4, buffer);
		const bool isStealSuccessRateToggled =
			buffer[0] != STEAL_CHANCE_ORIGINAL_0 ||
			buffer[1] != STEAL_CHANCE_ORIGINAL_1 ||
			buffer[2] != STEAL_CHANCE_ORIGINAL_2 ||
			buffer[3] != STEAL_CHANCE_ORIGINAL_3;


		printf("\n");
		printf("1) toggle steal success rate (%c)\n", isStealSuccessRateToggled ? 'y' : 'n');
		printf("e(x)it\n");
		const uint8_t c = getchar();
		if (c == 'x') {
			break;
		}

		if (c != '\n') {
			getchar(); // swallow newline
		}

		switch (c) {
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
			}
			default:
				break;
		}
	}
}
