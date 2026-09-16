make clean
./clean_coreutils.sh

python -m venv ./wllvm_venv
./wllvm_venv/bin/pip install wllvm
realpath ./wllvm_venv/bin/wllvm
export LLVM_COMPILER=clang
export CC=$(realpath ./wllvm_venv/bin/wllvm)
export CFLAGS="-O0 -fno-discard-value-names -fno-inline -Wc23-extensions"

git clone https://github.com/coreutils/coreutils
cd ./coreutils
git reset --hard 40bf1591bb4362fa91e501bcec7c2029c5f65a43
./bootstrap
patch --follow-symlinks -p1 < ../coreutils-9.2-patch.diff

./configure --program-suffix=-8.4
sudo rm -R .git/
rm ./.gitignore
echo * > ./.gitignore
rm -R ./gnulib/.git
make
