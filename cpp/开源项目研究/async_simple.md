<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-19 16:30:15
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-20 16:54:52
 * @FilePath: \note\note\cpp\开源项目研究\async_simple.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->

# 目录结构
utils:
     Queue.h:一个同步队列，大量使用移动语义避免拷贝，提供了try类型的出队操作
     Condition.h: 通过信号量模拟的条件变量,如果不支持信号量则用封通过锁和标准条件变量封装了pv操作
     move_only_function.h:一个只支持移动语义的可调用对象
     
     