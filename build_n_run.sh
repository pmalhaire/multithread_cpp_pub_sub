#!/bin/bash
set -e
cmake -B build
make -C build
./build/test_pub_sub
