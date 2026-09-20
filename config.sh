#!/bin/bash

sudo apt update

sudo apt intall -y python3 python3-pip make git wget rsync

sudo apt install \
  llvm-22 llvm-22-dev \
  clang-22 clang-tools-22 \
  libclang-22-dev lld-22 cmake ninja-build

./create_coreutils.sh
