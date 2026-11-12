#!/bin/bash
# build_tools.sh  build the C++ helper tools into ~/pbsd-tools
HERE=$(cd "$(dirname "$0")" && pwd); mkdir -p ~/pbsd-tools
g++ -O2 -std=c++17 -o ~/pbsd-tools/tuslice "$HERE/tuslice.cpp"
g++ -O2 -std=c++17 -o ~/pbsd-tools/acsl2bv "$HERE/acsl2bv.cpp"
