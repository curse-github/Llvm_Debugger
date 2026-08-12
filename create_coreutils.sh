make clean
./clean_coreutils.sh
make ./out/libClangPlugin.so
#export CLANGPLUGINARGS="-Xclang -load -Xclang $(realpath ./out/libClangPlugin.so) -Xclang -add-plugin -Xclang save-func-parms"
export CLANGPLUGINARGS=""
# preserves function parameter names, and will optimize code, but not completely optimize out functions
export COMPILEARGS="-O0 -fno-discard-value-names -fno-inline"

python -m venv ./wllvm_venv
./wllvm_venv/bin/pip install wllvm
realpath ./wllvm_venv/bin/wllvm
export LLVM_COMPILER=clang
export CC=$(realpath ./wllvm_venv/bin/wllvm)

git clone https://github.com/coreutils/coreutils -v
cd ./coreutils
./bootstrap
export CFLAGS="$CLANGPLUGINARGS $COMPILEARGS"
./configure
sudo rm -R .git/
rm ./.gitignore
echo * > ./.gitignore
sudo rm -R ./gnulib/.git
make
