#!/bin/bash

sudo apt update

sudo apt intall -y python3 python3-pip make git wget rsync

sudo apt install -y \
  build-essential autoconf automake autopoint \
  bison gettext gperf texinfo git rsync xz-utils

./create_coreutils.sh
