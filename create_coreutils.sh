python -m venv ./wllvm_venv
./wllvm_venv/bin/pip install wllvm
export LLVM_COMPILER=clang
realpath ./wllvm_venv/bin/wllvm
export CC=$(realpath ./wllvm_venv/bin/wllvm)
git clone https://github.com/coreutils/coreutils -v
cd ./coreutils
./bootstrap
# preserves function parameter names, and will optimize code, but not completely optimize out functions
export CFLAGS="-fno-inline -fno-discard-value-names -O3"
./configure
sudo rm -R .git/
rm ./.gitignore
echo * > ./.gitignore
sudo rm -R ./gnulib/.git
make
