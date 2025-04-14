#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="build"
EXECUTABLE="tests"
CLEAN=0
FILTER=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=1
            shift
            ;;
        --filter)
            FILTER="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--clean] [--filter <pattern>]"
            exit 1
            ;;
    esac
done

if [[ $CLEAN -eq 1 ]]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake ..
cmake --build . -- -j$(nproc)

if [[ -n "$FILTER" ]]; then
    ./$EXECUTABLE "$FILTER"
else
    ./$EXECUTABLE
fi
