if [ ! -d "./CrystalCore/" ];then git submodule add https://github.com/RetliveAdore/CrystalCore;fi
if [ ! -d "./CrystalCore/" ];then git submodule add https://github.com/RetliveAdore/CrystalAlgorithms;fi
if [ ! -d "./CrystalCore/" ];then git submodule add https://github.com/RetliveAdore/CrystalThread;fi
git submodule update --init
git submodule update --remote
cp ./platform/linux/makefile ./makefile
make build