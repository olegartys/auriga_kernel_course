#!/bin/bash

export LD_LIBRARY_PATH="."

make clean
make

./test
