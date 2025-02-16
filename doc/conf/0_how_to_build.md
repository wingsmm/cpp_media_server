# 1. 编译
# 1.1 安装C++17
## 1.1.1 centos安装c++11
```markup
yum install centos-release-scl -y
yum install devtoolset-8-gcc* -y
source /opt/rh/devtoolset-8/enable
```
## 1.1.2 ubuntu安装c++11
```markup
apt-get install -y gcc-8 g++-8
```
### 1.1.3 bzip2
```markup
yum install -y bzip2
yum install -y bzip2-devel
```

## 1.2 编译方式

### 1.2.1 增量编译
直接执行:
```bash
./build.sh
```
这将保留之前的编译结果，只编译发生变化的文件。

### 1.2.2 完全重新编译
执行:
```bash
./rebuild.sh
```
这将清除所有之前的编译结果，重新编译所有文件。

编译后的可执行程序都在 objs 目录下。

### 1.2.3 手动增量编译
如果你想手动控制编译过程，可以：
```bash
cd objs
cmake ..
make -j4
```



