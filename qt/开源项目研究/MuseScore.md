# 编译
```
安装 git/cmake/qt6
环境变量中先移除mingw的路径只保留msvc64的路径
并且下面的命令都在X64 Native 命令行执行

# 配置阶段
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug
 
# 编译阶段
cmake   --build build --parallel 16
 
# 安装阶段
cmake --install build --prefix install_dir
```