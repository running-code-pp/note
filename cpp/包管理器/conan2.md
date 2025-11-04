<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-09 01:19:12
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-14 20:22:37
 * @FilePath: \note\cpp\包管理器\conan2.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# 安装
[官方文档](https://docs.conan.org.cn/2/index.html)\
 
# 初始化
```bash
设置默认的profile\
conan profile detect --force\

查看profile文件的位置\
conan profile path default


conan install . --output-folder=build --build=missing --settings=build_type=Debug

cmake -S . -B build --preset conan-default
cmake --build build -j 16
```


# 安装依赖
conan install 依赖包/版本

# 查找依赖
conan search "boost*" --remote=conancenter


# 查看远程仓库列表
conan remote list

# 移除远程仓库
conan remote remove bincrafters