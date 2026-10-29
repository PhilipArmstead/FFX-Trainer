#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
cmake -S . -B build
cmake --build build --config Release
cmake --build build --config Release --target run
