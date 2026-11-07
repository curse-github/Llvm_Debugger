git clone https://github.com/AFLplusplus/AFLplusplus.git
cd AFLplusplus
git submodule update --init
make source-only
sudo make install
rm -R ./.git