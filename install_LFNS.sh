#!/usr/bin/env bash

printf "\n\nInstalling LFNS!\n"
## installing the toolbox
if [ ! -d ./build ]; then
    mkdir ./build
fi
cd build
printf "\nCalling CMake\n"
cmake -DCMAKE_INSTALL_PREFIX=$HOME/local/ ..
printf "\nCalling Make\n"
make
make install
cd ../