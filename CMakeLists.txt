# SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)
project(FFXTrainer VERSION 0.12.0 LANGUAGES C)

# Default build type for single-config generators only.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif ()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(FFXTrainer
	src/main.c
	src/memory.c
	src/process.c
	src/window.c
	src/callbacks.c
	src/callbacks.h
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

target_link_libraries(FFXTrainer PRIVATE PkgConfig::GTK4)

# Export runtime symbols on Linux so GtkBuilder can find G_MODULE_EXPORT callbacks
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# -rdynamic makes all symbols available to the dynamic linker for dlsym/lookup
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
endif ()

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

# --- Windows executable icon (MinGW) --------------------------------------
if (WIN32 AND (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang") AND NOT MSVC)
	# Put app.ico in your project root (same dir as CMakeLists.txt)
	set(APP_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/icons/app.ico")

	set(APP_RC "${CMAKE_CURRENT_BINARY_DIR}/appicon.rc")
	set(APP_OBJ "${CMAKE_CURRENT_BINARY_DIR}/appicon.obj")

	# Create the .rc file at build time
	file(WRITE "${APP_RC}" "IDI_ICON ICON \"${APP_ICON_ICO}\"\n")

	# Compile .rc -> .obj using windres
	add_custom_command(
		OUTPUT "${APP_OBJ}"
		COMMAND windres "${APP_RC}" -O coff -o "${APP_OBJ}"
		DEPENDS "${APP_ICON_ICO}" "${APP_RC}"
		VERBATIM
	)

	add_custom_target(appicon_obj ALL DEPENDS "${APP_OBJ}")

	# Link the icon object into the executable
	target_sources(FFXTrainer PRIVATE "${APP_OBJ}")
endif ()
