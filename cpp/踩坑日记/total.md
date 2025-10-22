<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-13 15:31:57
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-20 10:11:07
 * @FilePath: \note\note\cpp\踩坑日记\total.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# 初始化列表和成员声明顺序不同导致依赖错误
成员变量的构造顺序总是按照类中的声明顺序，而不是初始化列表中的顺序！


所以如果成员a的构造依赖成员b，类中的生命是 先b 后 a ，就算初始化列表中是先a 后 b也没用


# c++20format不能格式化thread_i
c++20 的format不能格式化std::thread_id\
c++23 才开始支持\
详见:https://runebook.dev/cn/docs/cpp/thread/thread/id/formatter

解决办法:
``` cpp
std::string thread_id_str() {
    std::ostringstream 
    oss << std::this_thread::get_id();
    return oss.str();
}

```

# thread::hardware_concurrency
返回的是cpu逻辑核心数，不是物理核心数，如果开启了超线程那么返回的是物理核心数的两倍


# 找不到msvcp100.dll
这是vc++的运行库，在下面的链接安装即可

https://learn.microsoft.com/zh-cn/cpp/windows/latest-supported-vc-redist?view=msvc-170
