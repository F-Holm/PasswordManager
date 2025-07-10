rm -fr ./build
git submodule sync
git submodule update --remote --recursive
git submodule update --init --recursive --checkout