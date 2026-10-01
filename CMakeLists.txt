# SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)
project(FFXTrainer VERSION 0.13.0 LANGUAGES C)

if (WIN32)
	enable_language(RC)
endif ()

# Default build type for single-config generators only.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif ()

set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(FFXTrainer
	src/main.c
)

if (WIN32)
	if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		target_link_options(FFXTrainer PRIVATE -mwindows)
	endif ()
endif ()

# Allow including headers using absolute paths relative to the project `src/` directory,
# e.g. #include "platform/platform.h"
target_include_directories(FFXTrainer PRIVATE
	${PROJECT_SOURCE_DIR}/src
)

# Keep binary name with hyphens, but use a CMake-safe target name.
set_target_properties(FFXTrainer PROPERTIES
	OUTPUT_NAME "FFX Trainer"
)

target_compile_features(FFXTrainer PRIVATE c_std_99)

target_compile_options(FFXTrainer PRIVATE
	$<$<C_COMPILER_ID:GNU,Clang,AppleClang>:
	-Wall
	-Wextra
	-Wpedantic
	-Wshadow
	-Wconversion
	-Wsign-conversion
	-Wstrict-prototypes
	-Wmissing-prototypes
	-Wno-missing-field-initializers
	>
	$<$<C_COMPILER_ID:MSVC>:
	/W4
	>
)

target_compile_definitions(FFXTrainer PRIVATE
	$<$<PLATFORM_ID:Windows>:ARCH_WIN>
	$<$<PLATFORM_ID:Darwin>:ARCH_MACOS>
	$<$<PLATFORM_ID:Linux>:ARCH_LINUX>
	$<$<CONFIG:Debug>:DEBUG>
	REPO_ROOT_DIR="${REPO_ROOT}"
	LAYOUTS_DIR="${REPO_ROOT}/layouts"
)

add_custom_target(info
	COMMAND ${CMAKE_COMMAND} -E echo "Generator: ${CMAKE_GENERATOR}"
	COMMAND ${CMAKE_COMMAND} -E echo "Config: $<CONFIG>"
	COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}"
	COMMAND ${CMAKE_COMMAND} -E echo "Target: $<TARGET_FILE:FFXTrainer>"
)

add_custom_target(run
	COMMAND $<TARGET_FILE:FFXTrainer>
	DEPENDS FFXTrainer
	WORKING_DIRECTORY $<TARGET_FILE_DIR:FFXTrainer>
	USES_TERMINAL
)

# --- Windows executable icon ----------------------------------------------
if (WIN32)
	set(APP_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/icons/app.ico")
	set(APP_RC "${CMAKE_CURRENT_BINARY_DIR}/appicon.rc")
	configure_file("${APP_ICON_ICO}" "${CMAKE_CURRENT_BINARY_DIR}/app.ico" COPYONLY)

	file(WRITE "${APP_RC}"
		"#define IDI_APP_ICON 1\nIDI_APP_ICON ICON \"app.ico\"\n"
	)

	target_sources(FFXTrainer PRIVATE "${APP_RC}")
endif ()
