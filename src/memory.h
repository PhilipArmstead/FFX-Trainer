#pragma once

#include <stdint.h>
#include <unistd.h>


static inline void readFromMemory(const int fd, const uint64_t address, const int length, uint8_t *bytes) {
	lseek(fd, address, SEEK_SET);
	read(fd, bytes, length);
}

static inline uint8_t readByte(const int fd, const uint64_t address) {
	uint8_t byte;
	lseek(fd, address, SEEK_SET);
	read(fd, &byte, 1);

	return byte;
}

static inline void writeToMemory(
	const int fd,
	const uint32_t address,
	const int length,
	const uint8_t *bytes
) {
	lseek(fd, address, SEEK_SET);
	write(fd, bytes, length);
}
