#!/bin/bash

sudo apt update

sudo apt intall -y python3 python3-pip make git wget rsync

sudo apt install -y \
  build-essential autoconf automake autopoint \
  bison gettext gperf texinfo git rsync xz-utils \
  lsb-release wget curl software-properties-common gnupg

sudo apt install \
  llvm-22 llvm-22-dev \
  clang-22 clang-tools-22 \
  libclang-22-dev lld-22


./create_coreutils.sh
