#!/bin/bash

for i in `find ./ -name *.cpp` ; do
  clang-format $i -i;
done

for i in `find ./ -name *.hpp` ; do
  clang-format $i -i;
done
