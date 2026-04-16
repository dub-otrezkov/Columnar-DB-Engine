#!/usr/bin/env bash
set -e

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$REPO_DIR/build"

mkdir -p "$BUILD_DIR"
cmake -S "$REPO_DIR/src" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j"$(nproc)" --target clickbench columnar_cli
