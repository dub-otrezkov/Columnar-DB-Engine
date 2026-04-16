#!/usr/bin/env bash
set -e

sudo apt-get update
sudo apt-get install -y cmake ninja-build libgtest-dev g++ libgoogle-perftools-dev

# libgtest-dev installs sources only — build and install the libs
sudo cmake -S /usr/src/googletest -B /tmp/gtest-build -G Ninja
sudo cmake --build /tmp/gtest-build
sudo cmake --install /tmp/gtest-build
