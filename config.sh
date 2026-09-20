#!/bin/bash

sudo apt update

sudo apt intall -y python3 python3-pip make git wget rsync

sudo apt install -y \
  build-essential autoconf automake autopoint \
  bison gettext gperf texinfo git rsync xz-utils \
  lsb-release wget curl software-properties-common gnupg

sudo apt install -y \
  llvm-21-dev llvm-21-tools \
  clang-21 clang-tools-21 \
  libclang-21-dev lld-21

./create_coreutils.sh
