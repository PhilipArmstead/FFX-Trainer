cmake_minimum_required(VERSION 3.14)
project(Game LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set build type (default to Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Directories
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(RAYLIB_DIR ${LIB_DIR}/raylib)
set(RAYLIB_LIB ${RAYLIB_DIR}/lib/libraylib.a)
set(RAYLIB_INCLUDE ${RAYLIB_DIR}/include)

# Raylib settings
set(RAYLIB_VERSION 4.5.0)

if(WIN32)
    set(RAYLIB_URL "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_Win64_mingw-w64.zip")
    set(EXTRA_LIBS opengl32 gdi32 winmm)
else()
    set(RAYLIB_URL "https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz")
    set(EXTRA_LIBS GL m pthread dl rt X11)
endif()

# Ensure lib directory exists
file(MAKE_DIRECTORY ${LIB_DIR})

# Download and extract Raylib if not found
if(NOT EXISTS ${RAYLIB_DIR})
    message(STATUS "Downloading Raylib...")
    file(DOWNLOAD ${RAYLIB_URL} ${CMAKE_BINARY_DIR}/raylib.tar.gz SHOW_PROGRESS)

    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/raylib.tar.gz
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    file(GLOB EXTRACTED_DIR "${CMAKE_BINARY_DIR}/raylib-*")
    execute_process(COMMAND ${CMAKE_COMMAND} -E rename ${EXTRACTED_DIR} ${RAYLIB_DIR})
endif()

# Build Raylib if the library file is missing
if(NOT EXISTS ${RAYLIB_LIB})
    message(STATUS "Building Raylib...")
    execute_process(
      COMMAND make
      WORKING_DIRECTORY ${RAYLIB_DIR}/src
    )
endif()

# Add source files
file(GLOB SOURCES src/*.c)

# Define the executable
add_executable(game ${SOURCES})

# Include Raylib headers from lib/raylib/include
target_include_directories(game PRIVATE ${RAYLIB_INCLUDE})

# Link Raylib and required system libraries
target_link_libraries(game PRIVATE ${RAYLIB_LIB} ${EXTRA_LIBS})

# Enable debugging symbols in Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(game PRIVATE -g)
endif()
