#!/bin/bash
set -v

clang++ --std=c++17 -O3 -Wall -o indexer util.cpp indexer.cpp

clang++ --std=c++17 -O3 -Wall -o searcher util.cpp searcher.cpp