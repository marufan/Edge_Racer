#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

if [ ! -d "build" ]; then
    echo "Configuring CMake..."
    cmake -B build -DCUSTOMIZE_BUILD=ON -DBUILD_EXAMPLES=OFF
fi

echo "Building EdgeRacer..."
cmake --build build -j$(nproc)

echo "Starting EdgeRacer..."
./build/EdgeRacer "$@"
