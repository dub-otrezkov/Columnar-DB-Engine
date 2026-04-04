#!/usr/bin/env bash
set -e

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$REPO_DIR/build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$REPO_DIR/src"
make -j"$(nproc)"
