
export LLVM_COMPILER=clang
export CC=$(realpath ./wllvm_venv/bin/wllvm)
make libClangPlugin.so
# preserves function parameter names, and will optimize code, but not completely optimize out functions
export CFLAGS="-Xclang -load -Xclang $(realpath ./out/libClangPlugin.so) -Xclang -add-plugin -Xclang save-func-parms -fno-inline -fno-discard-value-names"

python -m venv ./wllvm_venv
./wllvm_venv/bin/pip install wllvm
realpath ./wllvm_venv/bin/wllvm
git clone https://github.com/coreutils/coreutils -v
cd ./coreutils
./bootstrap
./configure
sudo rm -R .git/
rm ./.gitignore
echo * > ./.gitignore
sudo rm -R ./gnulib/.git
make
