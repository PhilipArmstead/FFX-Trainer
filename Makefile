TARGET_EXEC := FFXTrainer
BUILD_DIR := ./build
SRC_DIR := ./src

# Raylib config
RAYLIB_DIR := external/raylib
RAYLIB_LIB := $(RAYLIB_DIR)/src/libraylib.a
RAYLIB_INCLUDE := $(RAYLIB_DIR)/src

# Clone a specific version of Raylib (or latest master)
RAYLIB_REPO := https://github.com/raysan5/raylib.git
RAYLIB_TAG := 5.5

# Find all the C files
SRCS := $(shell find $(SRC_DIR) -name '*.c')

# Object & dependency files
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Include directories (your src dirs + Raylib headers)
INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) -I$(RAYLIB_INCLUDE)

CC := clang
CCFLAGS += -std=gnu99 -Wall -Werror -Wno-pointer-sign $(INC_FLAGS)

# Link Raylib statically from our local build
LDFLAGS := $(LDFLAGS) -lm $(RAYLIB_LIB)

# ---------------------------------------------------------------------------
# Raylib download and build
# ---------------------------------------------------------------------------
# Clone Raylib repo if missing
$(RAYLIB_DIR):
	@echo "Cloning Raylib..."
	git clone --depth 1 --branch $(RAYLIB_TAG) $(RAYLIB_REPO) $(RAYLIB_DIR)

# Build Raylib static library
$(RAYLIB_LIB): | $(RAYLIB_DIR)
	@echo "Building Raylib..."
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_DESKTOP

# ---------------------------------------------------------------------------
# Project build
# ---------------------------------------------------------------------------
all: $(RAYLIB_LIB)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) $(SRCS) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)
	@cp -r assets $(BUILD_DIR)

release: CCFLAGS += -O3
release: all

release-with-debug: CCFLAGS += -DDEBUG -g -fsanitize=address,undefined -O3
release-with-debug: all

debug: CCFLAGS += -DDEBUG -g -fsanitize=address,undefined
debug: all

clean:
	rm -rf $(BUILD_DIR)

clean_raylib:
	$(MAKE) -C $(RAYLIB_DIR)/src clean || true

clean_all: clean clean_raylib
	rm -rf $(RAYLIB_DIR)

.PHONY: all release debug clean clean_raylib clean_all
-include $(DEPS)
