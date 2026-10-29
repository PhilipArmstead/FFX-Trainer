# SPDX-FileCopyrightText: © 2026 Phil Armstead <philarmstead@mailbox.org>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)
project(FFXTrainer VERSION 0.13.0 LANGUAGES C)

if (NOT WIN32)
	message(FATAL_ERROR "FFX Trainer is Windows-only. Use a Windows compiler or a Windows cross-compilation toolchain.")
endif ()
enable_language(RC)

# Default build type for single-config generators only.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif ()

add_executable(FFXTrainer WIN32
	src/main.c
	src/trainer.c
)

target_link_libraries(FFXTrainer PRIVATE user32 gdi32)

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
	$<$<CONFIG:Debug>:DEBUG>
)

include(CTest)
if (BUILD_TESTING)
	add_executable(trainer_test tests/trainer_test.c)
	add_executable(trainer_memory_test tests/trainer_memory_test.c)
	add_executable(trainer_ui_test tests/trainer_ui_test.c)
	target_link_libraries(trainer_memory_test PRIVATE user32 gdi32)
	target_link_libraries(trainer_ui_test PRIVATE user32 gdi32)
	foreach (test_target trainer_test trainer_memory_test trainer_ui_test)
		target_compile_features(${test_target} PRIVATE c_std_99)
		target_compile_options(${test_target} PRIVATE
			$<$<C_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>
			$<$<C_COMPILER_ID:MSVC>:/W4>
		)
	endforeach ()
	add_test(NAME trainer_logic COMMAND trainer_test)
	add_test(NAME trainer_process_memory COMMAND trainer_memory_test)
	add_test(NAME trainer_native_ui COMMAND trainer_ui_test)
	set_tests_properties(trainer_logic trainer_process_memory trainer_native_ui PROPERTIES TIMEOUT 30)
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
