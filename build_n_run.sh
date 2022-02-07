#!/bin/bash
set -e
g++ --std=c++17 -o test  main.cpp -lpthread 
./test