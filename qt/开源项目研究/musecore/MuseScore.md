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

# 源码分析
MuseScore\src\app\main.cpp：做了很多平台兼容的设置，比如主题/控件风格/opengl的buglist\滚轮减速度等


framework/global/thirdparty:
这里面提供了几个直接通过源码集成的第三方实用组件：
- kors_async: 在主线程中执行异步任务
- kors_logger: 日志
- kors_modularity: 基于模块的简易依赖注入框架（每个模块一个实例)
- kors_msgpack: 对于标准类型进行二进制序列化和反序列化
- kors_profiler: 性能分析工具
- picojson: json解析
- pugixml: xml解析
- utfcpp: utf字符串编码工具库


**framework：核心架构**
- global: 全局配置，模块管理
- audio:封装了各种系统/web 底层音频接口
- action: 菜单动作之类的
- dockwindow: dock栏
- diagnostics: 捕获异常崩溃
- languages: 多语言实现(切换语言需要重启)
- network: 基于qwebsocket封装了一下- websocketapi
- shortcuts:快捷键管理
- stubs: 各个模块的空实现，不影响整体运行，适合隔离模块用于测试
- ui： ui框架/主题
- uicomponents: ui控件封装
- update: 更新（检查更新/下载release程序)
- workspace: 工作空间管理
  
**appshell: 用户界面层**

**context:上下文状态管理**