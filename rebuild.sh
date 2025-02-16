#!/bin/bash

# 删除并重新创建 objs 目录
if [ -d "objs" ]; then
    rm -rf objs
fi
mkdir objs

cd objs

# 重新运行 cmake 和 make
cmake ..
make -j4 