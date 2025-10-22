# cmake生成compile_commands.json
**compile_commands.json格式**
```text
是一个 json 文件， 只包含一个数组， 数组元素是 command object 类型。 每个 command object 包含以下字段: ([参考链接1])

directory: 字符串， 命令执行的目录。 绝对路径。 在 file 和 command 中指定的路径， 要么是绝对路径， 要么是相对于 directory 的路径。
file: 字符串， 源代码文件。 绝对路径， 或相对于 directory 的路径.
arguments: 或 command: 编译命令。
arguments 是一个字符串数组， 每个元素是命令行参数
command 是一个字符串， 整个命令行. 对于需要双引号包裹的参数， 需要使用 \" 转义. 例如 -DANDROID_ABI=\"arm64-v8a\".
推荐用 arguments， 因为它更容易解析和处理
output: 字符串， 编译输出文件. 可选字段.
```

**生成方法**
- 在cmakelists.txt中添加：set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
- 在cake configure阶段命令行传参：cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON <其他参数>


# cmake 常用宏
