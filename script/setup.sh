#!/usr/bin/env bash
set -e

sudo apt-get update
sudo apt-get install -y cmake ninja-build libgtest-dev g++ libgoogle-perftools-dev

# libgtest-dev installs sources only — build and install the libs
cd /usr/src/googletest
sudo cmake -B /tmp/gtest-build -G Ninja .
sudo cmake --build /tmp/gtest-build
sudo cmake --install /tmp/gtest-build