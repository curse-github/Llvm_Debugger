make clean
./clean_coreutils.sh

python -m venv ./wllvm_venv
./wllvm_venv/bin/pip install wllvm
realpath ./wllvm_venv/bin/wllvm
export LLVM_COMPILER=clang
export CC=$(realpath ./wllvm_venv/bin/wllvm)

git clone https://github.com/coreutils/coreutils -v
cd ./coreutils
./bootstrap
export CFLAGS="-O0 -fno-discard-value-names -fno-inline -Wc23-extensions"
./configure
sudo rm -R .git/
rm ./.gitignore
echo * > ./.gitignore
sudo rm -R ./gnulib/.git
make
