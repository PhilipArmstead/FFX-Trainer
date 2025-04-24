cmake_minimum_required(VERSION 3.14)
project(FFXTrainer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

# Find or fetch Raylib
include(FetchContent)
fetchcontent_declare(
	raylib
	GIT_REPOSITORY https://github.com/raysan5/raylib.git
	GIT_TAG 5.5
)

# Check if Raylib is already available
find_package(raylib QUIET)
if (NOT raylib_FOUND)
	message(STATUS "Raylib not found, fetching and building it...")
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as a static library" FORCE)
	fetchcontent_makeavailable(raylib)
endif ()

add_compile_options(-Wall -Wextra -Werror -Wno-missing-field-initializers)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options(-Wno-unused-parameter)
endif ()

# Gather all .c source files
file(GLOB_RECURSE SRC_FILES "src/*.c")

# Set up executable targets
add_executable(main ${SRC_FILES})
target_include_directories(main PRIVATE ./include ${raylib_SOURCE_DIR}/src)
target_link_libraries(main PRIVATE raylib)

# Platform-specific settings
if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)
	target_compile_definitions(main PRIVATE PLATFORM_OSX)
	target_link_libraries(main PRIVATE "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
elseif (WIN32)
	target_compile_definitions(main PRIVATE PLATFORM_WINDOWS)
else ()
	target_compile_definitions(main PRIVATE PLATFORM_LINUX)
endif ()

message(STATUS "CMake setup complete. Run 'make' or 'cmake --build .' to compile.")