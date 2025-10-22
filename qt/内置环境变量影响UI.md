- [UI风格](#ui风格)
  - [设置平台主题风格，QT\_QPA\_PLATFORMTHEME](#设置平台主题风格qt_qpa_platformtheme)
  - [设置滚轮减速度，QT\_QUICK\_FLICKABLE\_WHEEL\_DECELERATION](#设置滚轮减速度qt_quick_flickable_wheel_deceleration)
  - [禁用qml缓存](#禁用qml缓存)
  - [控件风格，QT\_STYLE\_OVERRIDE](#控件风格qt_style_override)
  - [Opengl优化，QT\_OPENGL\_BUGLIST](#opengl优化qt_opengl_buglist)
- [qml js引擎GC策略控制](#qml-js引擎gc策略控制)

# UI风格
## 设置平台主题风格，QT_QPA_PLATFORMTHEME 
设置平台主题，主要针对linux发行版

```
文件对话框样式
颜色、图标、字体
原生控件风格（如按钮、滚动条）
是否集成系统主题（如 KDE、GNOME）
```

| 值              | 说明                                                         |
|-----------------|--------------------------------------------------------------|
| kde             | 使用 KDE Plasma 桌面的主题（依赖 KF5）                      |
| gnome           | 使用 GNOME 桌面的主题（已弃用，现代系统用 gtk3）            |
| gtk2            | 使用 GTK+ 2 主题（较老）                                     |
| gtk3            | 推荐，使用 GTK+ 3 主题，外观与 GTK 应用一致                 |
| generic         | Qt 内置的通用主题（默认）                                    |
| qgnomeplatform  | 现代 GNOME 集成（需安装 qt5-wayland 或 qt6-wayland 相关包） |


## 设置滚轮减速度，QT_QUICK_FLICKABLE_WHEEL_DECELERATION
qtquick中的滚动区域默认减速度不太跟手（结束滚动之后没有立马停下来)

```
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (!qEnvironmentVariableIsSet("QT_QUICK_FLICKABLE_WHEEL_DECELERATION"))
    {
        // 设置滚轮滚动的减速度，值越大滚动会更快停下更跟手，px/s^2
        qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "5000");
    }
#endif
```

## 禁用qml缓存
qml引擎在加载qml或者js文件的时候会编译成字节码然后运行，为了防止每次加载都重新编译所以引入了磁盘缓存机制，禁用可以防止有时修改不会立马生效
```
    qputenv("QML_DISABLE_DISK_CACHE", "true");
```

## 控件风格，QT_STYLE_OVERRIDE

只针对于widget和qtquickcontrols2,对于自绘控件无效
- Fusion（跨平台现代风格，推荐，最统一）
- Windows（Windows 原生风格，仅 Windows 下可用）
- WindowsVista（Vista 风格，仅 Windows 下可用）
- macOS（macOS 原生风格，仅 macOS 下可用）
- gtk2/gtk3（Linux 下可用，依赖 GTK 库）
- Material（Qt Quick Controls 2 专用，现代移动风格）
- Universal（Qt Quick Controls 2 专用，Windows 10 风格）
- Imagine（Qt Quick Controls 2 专用，图片驱动风格）
- Basic（Qt Quick Controls 2 专用，基础风格）
- WindowsNative（部分 Qt 版本有）


## Opengl优化，QT_OPENGL_BUGLIST
**qt6** 之后支持通过设置QT_OPENGL_BUGLIST环境变量来规避Opengl驱动或者硬件问题，主要针对于windows

```
#ifdef Q_OS_WIN
    // NOTE: There are some problems with rendering the application window on some integrated graphics processors
    //       see https://github.com/musescore/MuseScore/issues/8270
    if (!qEnvironmentVariableIsSet("QT_OPENGL_BUGLIST"))
    {
        qputenv("QT_OPENGL_BUGLIST", ":/resources/win_opengl_buglist.json");
    }
#endif
```

参考MuseScore中规避的Opengl相关问题以及策略
``` json
{
    "name": "Qt built-in GPU driver blacklist",
    "version": "5.6",
    "entries": [
        {
            "id": 1,
            "description": "Desktop OpenGL is unreliable on some Intel HD laptops (QTBUG-43263)",
            "vendor_id": "0x8086",
            "device_id": [ "0x0A16" ],
            "os": {
                "type": "win"
            },
            "driver_version": {
                "op": "<=",
                "value": "10.18.10.3277"
            },
            "features": [
                "disable_desktopgl"
            ]
        },
        {
            "id": 2,
            "description": "Intel Q965/Q963 - GMA 3000 has insufficient support of opengl and directx",
            "vendor_id": "0x8086",
            "device_id": [ "0x2992" ],
            "os": {
                "type": "win"
            },
            "features": [
                "disable_desktopgl",
                "disable_angle"
            ]
       },
       {
           "id": 3,
           "description": "No OpenGL on Intel G33/G31 (QTBUG-47522)",
           "vendor_id": "0x8086",
           "device_id": [ "0x29C2" ],
           "os": {
               "type": "win"
           },
           "features": [
               "disable_desktopgl"
           ]
       },
       {
           "id": 4,
          "description": "Intel HD Graphics 3000 crashes when initializing the OpenGL driver (QTBUG-42240)",
          "vendor_id": "0x8086",
          "device_id": [ "0x0102", "0x0106", "0x010A", "0x0112", "0x0116", "0x0122", "0x0126" ],
          "os": {
              "type": "win"
          },
          "features": [
              "disable_desktopgl"
          ]
       },
       {
           "id": 5,
           "description": "Intel GMA 3150 (QTBUG-43243), Mobile Intel 945GM (QTBUG-47435) crash",
           "vendor_id": "0x8086",
           "device_id": [ "0xA001", "0xA011", "0x27A0" ],
           "os": {
               "type": "win"
           },
           "features": [
               "disable_desktopgl", "disable_angle"
           ]
        },
        {
           "id": 6,
           "description": "Intel(R) HD Graphics 4000 / 5500 cause crashes on orientation changes in fullscreen mode (QTBUG-49541)",
           "vendor_id": "0x8086",
           "device_id": [ "0x0166", "0x1616" ],
           "os": {
               "type": "win"
           },
           "features": [
               "disable_rotation"
           ]
        },
        {
           "id": 7,
           "description": "AMD FirePro V5900 driver causes crashes in Direct3D on Windows.",
           "vendor_id": "0x1002",
           "device_id": ["0x6707"],
           "os": {
               "type": "win"
           },
           "features": [
               "disable_angle"
           ]
        },
        {
           "id": 8,
           "description": "Standard VGA: Insufficient support for OpenGL, D3D9 and D3D11",
           "vendor_id": "0x0000",
           "device_id": ["0x0000"],
           "os": {
               "type": "win"
           },
           "features": [
               "disable_desktopgl", "disable_d3d11", "disable_d3d9"
           ]
        },
        {
           "id": 9,
           "description": "Intel 945 crash (QTBUG-40991)",
           "vendor_id": "0x8086",
           "device_id": [ "0x27A2" ],
           "os": {
               "type": "win"
           },
           "features": [
               "disable_desktopgl"
           ]
        },
        {
          "id": 10,
          "description": "Intel(R) HD Graphics IronLake (Arrandale) crashes on makeCurrent QTBUG-53888",
          "vendor_id": "0x8086",
          "device_id": [ "0x0046" ],
          "os": {
              "type": "win"
          },
          "features": [
              "disable_desktopgl"
          ]
        },
        {
          "id": 11,
          "description": "Intel driver version 8.15.10.1749 causes GPU process hangs (QTBUG-56360)",
          "vendor_id": "0x8086",
          "os": {
            "type": "win"
          },
          "driver_version": {
            "op": "=",
            "value": "8.15.10.1749"
          },
          "features": [
            "disable_desktopgl", "disable_d3d11", "disable_d3d9"
          ]
        },
        {
           "id": 12,
           "description": "Intel HD Graphics crash in conjunction with shader caches (QTBUG-64697) - disable for all Intel GPUs",
           "vendor_id": "0x8086",
           "os": {
               "type": "win"
           },
           "features": [
               "disable_program_cache"
           ]
        },
        {
           "id": 13,
           "description": "Disable DesktopGL on Windows with Mobile Intel(R) 4 Series Express Chipset Family graphics card (QTBUG-58772)",
           "vendor_id": "0x8086",
           "device_id": [ "0x2A42" ],
           "os": {
               "type": "win"
           },
           "features": [
               "disable_desktopgl"
           ]
       }
    ]
}


```


# qml js引擎GC策略控制
自 Qt 6.8 起，它默认以增量方式运行（除非 QV4_GC_TIMELIMIT 设置为 0）
但是可能不太成熟在某些系统上会造成异常行为所以可以禁用，(MuseScore禁用了增量GC)
```cpp
    if (!qEnvironmentVariableIsSet("MU_QV4_GC_TIMELIMIT"))
    {
        qputenv("QV4_GC_TIMELIMIT", "0");
    }
```


```
🔹 传统垃圾回收（非增量）的问题
在传统的 “全停顿”（Stop-the-world） 垃圾回收中：

当 GC 触发时，整个 JavaScript 引擎会暂停执行。
GC 遍历所有对象，标记并清理不再使用的内存。
这个过程可能持续几十甚至上百毫秒。
结果：UI 卡顿、动画掉帧、用户体验差。
❌ 问题：一次回收时间越长，卡顿越明显。

✅ 增量式 GC 的解决方案
增量式垃圾回收（Incremental GC） 将一次大的 GC 任务拆分成多个小任务，分多次执行：

每次只执行一小部分垃圾回收工作。
执行完一小步后，立即交还控制权给 JS 引擎，继续执行脚本或渲染 UI。
下次再继续执行下一小步，直到整个回收完成。
```