export LLVM_COMPILER=clang
cd ./coreutils
CC=../venv/bin/wllvm ./configure
CC=../venv/bin/wllvm make
