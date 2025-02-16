#!/bin/bash

# 如果 objs 目录不存在则创建
if [ ! -d "objs" ]; then
    mkdir objs
fi

cd objs

# 只有当 CMakeCache.txt 不存在时才运行 cmake
if [ ! -f "CMakeCache.txt" ]; then
    cmake ..
fi

# 增量编译
make -j4