# .vscode中的文件
## task.json
任务配置：定义可以在vscode中运行的自定义任务，比如编译、打包、清理,在lauch.json中preLaunchTask调用
```
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Configure",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-B",
                "build",
                "-G",
                "Visual Studio 17 2022",
                "-A",
                "x64",
                "-DCMAKE_BUILD_TYPE=Debug",
                "-DMUE_BUILD_ENGRAVING_TESTS=ON"
            ],
            "group": "build",
            "problemMatcher": []
        },
        {
            "label": "CMake Build",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "build",
                "--config",
                "Debug"
            ],
            "group": "build",
            "problemMatcher": [
                "$msCompile"
            ]
        },
        {
            "label": "CMake Clean",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "build",
                "--target",
                "clean"
            ],
            "group": "build",
            "problemMatcher": []
        }
    ]
}
```


## launch.json
定义如何启动和调试程序

- 指定执行文件：program
- 设置调试器类型: type(cppdbg/cppvsdbg)
- 设置入参: args
- 设置工作目录: cwd
- 调试器路径: miDebuggerPath
- 启动之前的任务: preLaunchTask(tasks.json中定义的任务)
  

## c_cpp_properties.json
C/C++编辑器配置文件:仅仅针对于编辑器来说，不然头文件包含爆红虽然可能可以编译，
但是编辑器不认识


## 模板
[tasks.json](../resource_file/tasks.json)

[settings.json](../resource_file/settings.json)
 
[launch.json](../resource_file/launch.json) 
  
[c_cpp_properties.json](../resource_file/c_cpp_properties.json)