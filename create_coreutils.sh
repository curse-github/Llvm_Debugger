python -m venv ./wllvm_venv
./wllvm_venv/bin/pip install wllvm
export LLVM_COMPILER=clang
git clone https://github.com/coreutils/coreutils -v
cd ./coreutils
./bootstrap
export CC=../wllvm_venv/bin/wllvm
# preserves function parameter names, and will optimize code, but not completely optimize out functions
export CFLAGS="-fno-inline -fno-discard-value-names -O3"
./configure
make
