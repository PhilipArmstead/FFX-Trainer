#pragma once

#include <fcntl.h>
#include <stdlib.h>


static inline long openProcess() {
	FILE *fp = popen("pidof FFX.exe", "r");
	char path[64];
	fgets(path, sizeof(path), fp);
	const long pid = strtol(path, NULL, 10);
	fclose(fp);

	return pid;
}

static inline int getProcessFileDescriptor() {
	const long pid = openProcess();
	if (pid) {
		char memoryPath[32];
		sprintf(memoryPath, "/proc/%ld/mem", pid);
		return open(memoryPath, O_RDWR);
	}

	return -1;
}
